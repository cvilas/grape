//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <format>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <SDL3/SDL_camera.h>
#include <SDL3/SDL_main.h>

#include "grape/exception.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"
#include "grape/utils/format_ranges.h"

//-------------------------------------------------------------------------------------------------
// Camera frame publisher over IPC
// This application captures camera frames and publishes them over IPC for remote display.
// Use with camera_subscriber to view the frames remotely.
//-------------------------------------------------------------------------------------------------

namespace {

//=================================================================================================
/// Image frame data structure for IPC transmission
struct Frame {
  std::vector<uint8_t> pixels;
  int pitch{};
  int width{};
  int height{};
  uint32_t format{};  // SDL_PixelFormat stored as uint32_t for serialization
  std::chrono::system_clock::time_point timestamp;
};

//=================================================================================================
/// Serialization function for Frame
constexpr auto BUF_SIZE = (1920U * 1080U * 3U) + 1024U;
using OutStream = grape::serdes::OutStream<BUF_SIZE>;
using Serialiser = grape::serdes::Serialiser<OutStream>;

[[nodiscard]] auto serialise(Serialiser& ser, const Frame& frame) -> bool {
  return ser.pack(frame.pixels) and ser.pack(frame.pitch) and ser.pack(frame.width) and
         ser.pack(frame.height) and ser.pack(frame.format) and
         ser.pack(frame.timestamp.time_since_epoch().count());
}

//=================================================================================================
/// Encapsulates data and model for statistics calculations
class Statistics {
public:
  void compute();

private:
  static constexpr auto REPORT_INTERVAL = std::chrono::seconds(30);
  uint64_t frame_count_{ 0 };
  std::chrono::system_clock::time_point start_ts_;
};

//=================================================================================================
/// Handles camera frame acquisition and publishing
class CameraPublisher {
public:
  explicit CameraPublisher(const std::string& topic);
  auto init() -> bool;     //!< initialise the camera
  auto acquire() -> bool;  //!< acquire and publish a frame

private:
  static void printCameraSpecs(SDL_CameraID camera_id);
  static auto getCameraDrivers() -> std::vector<std::string>;

  std::unique_ptr<SDL_Camera, void (*)(SDL_Camera*)> camera_{ nullptr, SDL_CloseCamera };
  std::unique_ptr<grape::ipc::Publisher> publisher_;
  Statistics stats_;
  OutStream stream_buffer_;
};

//-------------------------------------------------------------------------------------------------
void Statistics::compute() {
  const auto ts = std::chrono::system_clock::now();
  if (frame_count_ == 0) {
    start_ts_ = ts;
  }
  frame_count_++;
  if (ts - start_ts_ > REPORT_INTERVAL) {
    const auto stop = std::chrono::system_clock::now();
    const auto dt = std::chrono::duration<double>(stop - start_ts_).count();
    const auto frame_rate = std::floor(static_cast<double>(frame_count_) / dt);
    grape::syslog::Info("[{} frames/s (avg. over {})]", frame_rate, REPORT_INTERVAL);
    frame_count_ = 0;
  }
}

//-------------------------------------------------------------------------------------------------
CameraPublisher::CameraPublisher(const std::string& topic) {
  const auto match_cb = [](const grape::ipc::Match& match) -> void {
    grape::syslog::Info("Publisher match: {} (entity: {})", toString(match.status),
                        toString(match.remote_entity));
  };

  publisher_ = std::make_unique<grape::ipc::Publisher>(topic, match_cb);
}

//-------------------------------------------------------------------------------------------------
auto CameraPublisher::init() -> bool {
  grape::syslog::Info("Available camera drivers: {}", getCameraDrivers());
  grape::syslog::Info("Using camera driver: {}", SDL_GetCurrentCameraDriver());

  auto camera_count = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
  auto camera_ids_deleter = [](SDL_CameraID* ptr) { SDL_free(ptr); };
  auto cameras_ids = std::unique_ptr<SDL_CameraID, decltype(camera_ids_deleter)>(
      SDL_GetCameras(&camera_count), camera_ids_deleter);
  if ((cameras_ids == nullptr) or (camera_count == 0)) {
    grape::syslog::Critical("No cameras found, or unable to enumerate cameras: {}", SDL_GetError());
    return false;
  }

  grape::syslog::Info("Found {} camera{}", camera_count, camera_count > 1 ? "s" : "");
  for (int i = 0; i < camera_count; ++i) {
    const auto id = cameras_ids.get()[i];
    printCameraSpecs(id);
  }

  grape::syslog::Info("Opening first camera");
  camera_.reset(SDL_OpenCamera(cameras_ids.get()[0], nullptr));
  if (camera_ == nullptr) {
    grape::syslog::Critical("Unable to open camera: {}", SDL_GetError());
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------------------
auto CameraPublisher::acquire() -> bool {
  auto* sdl_frame = SDL_AcquireCameraFrame(camera_.get(), nullptr /*timestamp*/);
  if (sdl_frame == nullptr) {
    return true;  // No frame available, but continue
  }

  stats_.compute();

  // Convert SDL surface to our Frame structure for IPC transmission
  static Frame frame;
  frame.pitch = sdl_frame->pitch;
  frame.width = sdl_frame->w;
  frame.height = sdl_frame->h;
  frame.format = static_cast<uint32_t>(sdl_frame->format);
  frame.timestamp = std::chrono::system_clock::now();

  // Copy pixel data
  const auto pixel_data_size =
      static_cast<size_t>(sdl_frame->pitch) * static_cast<size_t>(sdl_frame->h);
  frame.pixels.resize(pixel_data_size);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  std::memcpy(frame.pixels.data(), sdl_frame->pixels, pixel_data_size);

  // Serialize and publish the frame
  auto serializer = Serialiser(stream_buffer_);
  if (serializer.pack(frame)) {
    auto data_span = stream_buffer_.data();
    publisher_->publish(data_span);
    grape::syslog::Debug("Published frame {}x{} ({} bytes)", frame.width, frame.height,
                         data_span.size());
  } else {
    grape::syslog::Error("Failed to serialize frame");
  }

  SDL_ReleaseCameraFrame(camera_.get(), sdl_frame);
  return true;
}

//-------------------------------------------------------------------------------------------------
auto CameraPublisher::getCameraDrivers() -> std::vector<std::string> {
  const auto num_drivers = SDL_GetNumCameraDrivers();
  std::vector<std::string> drivers;
  for (int i = 0; i < num_drivers; ++i) {
    const char* driver_name = SDL_GetCameraDriver(i);
    drivers.emplace_back(driver_name);
  }
  return drivers;
}

//-------------------------------------------------------------------------------------------------
void CameraPublisher::printCameraSpecs(SDL_CameraID camera_id) {
  const auto* name = SDL_GetCameraName(camera_id);
  grape::syslog::Info(" {}", name);
  SDL_CameraSpec** specs = SDL_GetCameraSupportedFormats(camera_id, nullptr);
  if (specs == nullptr) {
    grape::syslog::Warn("Unable to get camera specs: {}", SDL_GetError());
    return;
  }
  grape::syslog::Info("  Supported specs:");
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  for (int i = 0; specs[i] != nullptr; ++i) {
    const auto* sp = specs[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    grape::syslog::Info("   {}x{}, {} FPS, {}", sp->width, sp->height,
                        sp->framerate_numerator / sp->framerate_denominator,
                        SDL_GetPixelFormatName(sp->format));
  }
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
  SDL_free(specs);
}

//-------------------------------------------------------------------------------------------------
struct LogFormatter {
  static auto format(const grape::log::Record& record) -> std::string {
    return std::format("[{}] [{:9s}] {}", record.timestamp, toString(record.severity),
                       record.message.cStr());
  }
};

//-------------------------------------------------------------------------------------------------
void setupLogging() {
  auto log_config = grape::log::Config{};
  log_config.sink = std::make_shared<grape::log::ConsoleSink<LogFormatter>>();
  log_config.threshold = grape::log::Severity::Info;
  grape::syslog::init(std::move(log_config));
}

}  // namespace

//-------------------------------------------------------------------------------------------------
auto main(int argc, char** argv) -> int {
  try {
    setupLogging();

    // Initialize IPC
    grape::ipc::init(grape::ipc::Config{});

    // Initialize SDL
    if (not SDL_Init(SDL_INIT_CAMERA)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return EXIT_FAILURE;
    }

    const auto* topic = (argc > 1) ?
                            argv[1] :  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                            "camera_frames";
    grape::syslog::Info("Publishing camera frames on topic: {}", topic);

    auto publisher = CameraPublisher(topic);
    if (not publisher.init()) {
      return EXIT_FAILURE;
    }

    grape::syslog::Info("Camera publisher started. Press Ctrl+C to exit.");

    while (grape::ipc::ok()) {
      if (not publisher.acquire()) {
        grape::syslog::Error("Failed to acquire frame");
        break;
      }

      // Small delay to prevent overwhelming the system
      static constexpr auto FRAME_DELAY_MS = 33;  // ~30 FPS
      std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_DELAY_MS));
    }

    grape::syslog::Info("Camera publisher shutting down");
    return EXIT_SUCCESS;

  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
