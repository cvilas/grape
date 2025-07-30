//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <atomic>
#include <chrono>
#include <cmath>
#include <format>
#include <memory>
#include <mutex>
#include <vector>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/ipc/subscriber.h"
#include "grape/log/syslog.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

//-------------------------------------------------------------------------------------------------
// Camera frame subscriber and display over IPC
// This application subscribes to camera frames over IPC and displays them.
// Use with camera_publisher to receive and display remote camera frames.
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
/// Deserialization function for Frame
using InStream = grape::serdes::InStream;
using Deserialiser = grape::serdes::Deserialiser<InStream>;

[[nodiscard]] auto deserialise(Deserialiser& des, Frame& frame) -> bool {
  int64_t timestamp_count{};
  bool result = des.unpack(frame.pixels) and des.unpack(frame.pitch) and des.unpack(frame.width) and
                des.unpack(frame.height) and des.unpack(frame.format) and
                des.unpack(timestamp_count);

  if (result) {
    frame.timestamp =
        std::chrono::system_clock::time_point(std::chrono::system_clock::duration(timestamp_count));
  }

  return result;
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
/// Handles image rendering and window management
class Display {
public:
  auto init() -> bool;                               //!< initialise the display
  [[maybe_unused]] void render(const Frame& frame);  //!< show frame

private:
  std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> window_{ nullptr, SDL_DestroyWindow };
  std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer*)> renderer_{ nullptr, SDL_DestroyRenderer };
  std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)> texture_{ nullptr, SDL_DestroyTexture };
};

//=================================================================================================
/// Encapsulation of processing pipeline
class Application {
public:
  Application(const std::string& topic);

  auto init() -> SDL_AppResult;
  auto iterate() -> SDL_AppResult;
  auto handleEvent(SDL_Event* event) -> SDL_AppResult;

private:
  void onFrameReceived(const grape::ipc::Sample& sample);

  std::unique_ptr<Display> display_;
  std::unique_ptr<grape::ipc::Subscriber> subscriber_;
  std::atomic<bool> new_frame_available_{ false };
  Frame latest_frame_;
  std::mutex frame_mutex_;
  Statistics stats_;
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
auto Display::init() -> bool {
  static constexpr auto DEFAULT_WIDTH = 640;
  static constexpr auto DEFAULT_HEIGHT = 480;

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  if (not SDL_CreateWindowAndRenderer("Camera Subscriber View", DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                      SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    grape::syslog::Critical("SDL_CreateWindowAndRenderer failed: {}", SDL_GetError());
    return false;
  }

  window_.reset(window);
  renderer_.reset(renderer);
  return true;
}

//-------------------------------------------------------------------------------------------------
void Display::render(const Frame& frame) {
  // Create or recreate texture if needed
  auto sdl_format = static_cast<SDL_PixelFormat>(frame.format);
  if (texture_ == nullptr || texture_->w != frame.width || texture_->h != frame.height ||
      texture_->format != sdl_format) {
    texture_.reset(SDL_CreateTexture(renderer_.get(), sdl_format, SDL_TEXTUREACCESS_STREAMING,
                                     frame.width, frame.height));
    if (texture_ == nullptr) {
      grape::syslog::Error("SDL_CreateTexture failed: {}", SDL_GetError());
      return;
    }
  }

  // Update texture with new frame data
  if (not SDL_UpdateTexture(texture_.get(), nullptr, frame.pixels.data(), frame.pitch)) {
    grape::syslog::Error("SDL_UpdateTexture failed: {}", SDL_GetError());
  }

  // Setup background colour
  constexpr auto BGCOLOR = std::array<float, 4>{ 0.4F, 0.6F, 1.0F, SDL_ALPHA_OPAQUE_FLOAT };
  if (not SDL_SetRenderDrawColorFloat(renderer_.get(), BGCOLOR.at(0), BGCOLOR.at(1), BGCOLOR.at(2),
                                      BGCOLOR.at(3))) {
    grape::syslog::Warn("SDL_SetRenderDrawColorFloat failed: {}", SDL_GetError());
  }

  if (not SDL_RenderClear(renderer_.get())) {
    grape::syslog::Warn("SDL_RenderClear failed: {}", SDL_GetError());
  }

  int dw = 0;
  int dh = 0;
  SDL_GetRenderOutputSize(renderer_.get(), &dw, &dh);
  auto dst = SDL_FRect{ .x = 0, .y = 0, .w = static_cast<float>(dw), .h = static_cast<float>(dh) };

  // Calculate view dimensions maintaining aspect ratio
  const auto src_w = static_cast<float>(texture_->w);
  const auto src_h = static_cast<float>(texture_->h);
  const auto scale_w = dst.w / src_w;
  const auto scale_h = dst.h / src_h;
  const auto scale = std::min(scale_w, scale_h);
  const auto new_w = src_w * scale;
  const auto new_h = src_h * scale;
  dst.x = (dst.w - new_w) / 2;
  dst.y = (dst.h - new_h) / 2;
  dst.w = new_w;
  dst.h = new_h;

  if (not SDL_RenderTexture(renderer_.get(), texture_.get(), nullptr, &dst)) {
    grape::syslog::Warn("SDL_RenderTexture failed: {}", SDL_GetError());
  }

  if (not SDL_RenderPresent(renderer_.get())) {
    grape::syslog::Warn("SDL_RenderPresent failed: {}", SDL_GetError());
  }
}

//-------------------------------------------------------------------------------------------------
Application::Application(const std::string& topic) {
  display_ = std::make_unique<Display>();

  const auto data_cb = [this](const grape::ipc::Sample& sample) -> void {
    this->onFrameReceived(sample);
  };

  const auto match_cb = [](const grape::ipc::Match& match) -> void {
    grape::syslog::Info("Subscriber match: {} (entity: {})", toString(match.status),
                        toString(match.remote_entity));
  };

  subscriber_ = std::make_unique<grape::ipc::Subscriber>(topic, data_cb, match_cb);
}

//-------------------------------------------------------------------------------------------------
void Application::onFrameReceived(const grape::ipc::Sample& sample) {
  try {
    if (new_frame_available_ == true) {
      return;
    }
    std::lock_guard<std::mutex> lock(frame_mutex_);
    auto in_stream = InStream(sample.data);
    auto deserializer = Deserialiser(in_stream);

    if (deserializer.unpack(latest_frame_)) {
      new_frame_available_ = true;
      stats_.compute();

      const auto latency = std::chrono::system_clock::now() - latest_frame_.timestamp;
      grape::syslog::Debug("Received frame {}x{} (latency: {})", latest_frame_.width,
                           latest_frame_.height, latency);
    } else {
      grape::syslog::Error("Failed to deserialize received frame");
    }
  } catch (const std::exception& e) {
    grape::syslog::Error("Exception in frame callback: {}", e.what());
  }
}

//-------------------------------------------------------------------------------------------------
auto Application::init() -> SDL_AppResult {
  if (not display_->init()) {
    return SDL_APP_FAILURE;
  }
  grape::syslog::Info("Camera subscriber initialized. Waiting for frames...");
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto Application::iterate() -> SDL_AppResult {
  if (new_frame_available_.exchange(false)) {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    display_->render(latest_frame_);
  }
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto Application::handleEvent(SDL_Event* event) -> SDL_AppResult {
  if (event->type == SDL_EVENT_QUIT) {
    grape::syslog::Info("Quit!");
    return SDL_APP_SUCCESS;
  }

  return SDL_APP_CONTINUE;
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
auto SDL_AppInit(void** appstate, int argc, char** argv) -> SDL_AppResult {
  try {
    setupLogging();

    // Initialize IPC
    grape::ipc::init(grape::ipc::Config{});

    if (not SDL_Init(SDL_INIT_VIDEO)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    const auto* topic =
        (argc > 1) ? argv[1] :
                     "camera_frames";  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    grape::syslog::Info("Subscribing to camera frames on topic: {}", topic);

    auto app = std::make_unique<Application>(topic);
    const auto result = app->init();
    if (result != SDL_APP_CONTINUE) {
      return result;
    }

    *appstate = app.get();

    // Transfer ownership to static storage for automatic cleanup on exit
    static auto app_holder = std::move(app);

    return SDL_APP_CONTINUE;

  } catch (...) {
    grape::Exception::print();
    return SDL_APP_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
  auto* app = static_cast<Application*>(appstate);
  return app->iterate();
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
  auto* app = static_cast<Application*>(appstate);
  return app->handleEvent(event);
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
  // Cleanup is handled automatically by the static app_holder destructor
}
