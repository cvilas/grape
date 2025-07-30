//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <chrono>
#include <mutex>

#include <SDL3/SDL_main.h>

#include "grape/camera/decompressor.h"
#include "grape/camera/display.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/raw_subscriber.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/statistics/sliding_mean.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 to display camera frames acquired by the corresponding publisher appln.
//-------------------------------------------------------------------------------------------------

namespace grape::camera {

//=================================================================================================
/// Encapsulates processing pipeline: [subscribe] -> [decompress] -> [display]
class Subscriber {
public:
  explicit Subscriber(const std::string& topic);

  auto iterate() -> SDL_AppResult;
  auto handleEvent(SDL_Event* event) -> SDL_AppResult;
  void saveImage();
  auto latency() const -> std::chrono::system_clock::duration;

private:
  void onReceivedSample(const ipc::Sample& sample);
  void onDecompressedFrame(const ImageFrame& frame);
  void onPublisherMatch(const ipc::Match& match);

  static constexpr auto STATS_WINDOW = 600U;
  std::atomic<float> latency_;
  statistics::SlidingMean<float, STATS_WINDOW> latency_accum_;

  std::mutex image_lock_;
  ImageFrame::Header image_header_;
  std::vector<std::byte> image_data_;

  Display display_;
  Decompressor decompressor_;
  ipc::RawSubscriber subscriber_;
  std::atomic_bool save_snapshot_ = false;
};

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(const std::string& topic)
  : decompressor_([this](const auto& frame) { onDecompressedFrame(frame); })
  , subscriber_(
        topic, [this](const auto& sample) { onReceivedSample(sample); },
        [this](const auto& match) { onPublisherMatch(match); }) {
}

//-------------------------------------------------------------------------------------------------
void Subscriber::onReceivedSample(const ipc::Sample& sample) {
  if (not decompressor_.decompress(sample.data)) {
    syslog::Error("Decompression failed!");
  }
}

//-------------------------------------------------------------------------------------------------
void Subscriber::onDecompressedFrame(const ImageFrame& frame) {
  const auto now = std::chrono::system_clock::now();
  const auto latency =
      std::chrono::duration_cast<std::chrono::duration<float>>(now - frame.header.timestamp)
          .count();
  latency_.store(latency_accum_.append(latency).mean, std::memory_order_relaxed);

  auto guard = std::lock_guard(image_lock_);
  if (image_data_.size() < frame.pixels.size_bytes()) {
    image_data_.resize(frame.pixels.size_bytes());
  }
  image_header_ = frame.header;
  std::ranges::copy(frame.pixels, image_data_.begin());
}

//-------------------------------------------------------------------------------------------------
void Subscriber::onPublisherMatch(const ipc::Match& match) {
  (void)this;
  syslog::Note("{} (entity: {})", toString(match.status), toString(match.remote_entity));
}

//-------------------------------------------------------------------------------------------------
void Subscriber::saveImage() {
  save_snapshot_ = true;
}

//-------------------------------------------------------------------------------------------------
auto Subscriber::latency() const -> std::chrono::system_clock::duration {
  return std::chrono::duration_cast<std::chrono::system_clock::duration>(
      std::chrono::duration<float>(latency_.load(std::memory_order_relaxed)));
}

//-------------------------------------------------------------------------------------------------
auto Subscriber::iterate() -> SDL_AppResult {
  static auto last_ts = std::chrono::system_clock::now();
  auto guard = std::lock_guard(image_lock_);
  const auto frame = grape::camera::ImageFrame{ .header = image_header_, .pixels = image_data_ };
  const auto now_ts = frame.header.timestamp;
  if (now_ts > last_ts) {
    last_ts = now_ts;
    display_.render(frame);
    if (save_snapshot_) {
      save_snapshot_ = false;
      const auto fname = std::format("snapshot_{:%FT%T}.bmp", std::chrono::system_clock::now());
      std::ignore = grape::camera::save(frame, fname);
    }
  }
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto Subscriber::handleEvent(SDL_Event* event) -> SDL_AppResult {
  if (event->type == SDL_EVENT_QUIT) {
    syslog::Info("Quit!");
    return SDL_APP_SUCCESS;
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.scancode == SDL_SCANCODE_S) {
      saveImage();
    }
    return SDL_APP_CONTINUE;
  }

  return SDL_APP_CONTINUE;
}
}  // namespace grape::camera

namespace {
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
auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
  try {
    setupLogging();
    auto ipc_config = grape::ipc::Config{};
    ipc_config.scope = grape::ipc::Config::Scope::Network;
    grape::ipc::init(std::move(ipc_config));

    if (not SDL_Init(SDL_INIT_VIDEO)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    // Parse command line arguments
    const auto args_opt = grape::conio::ProgramDescription("Camera viewer application")
                              .declareOption<std::string>("topic", "image stream topic", "/camera")
                              .parse(argc, const_cast<const char**>(argv));

    if (not args_opt.has_value()) {
      grape::syslog::Critical("Failed to parse command line arguments: {}",
                              toString(args_opt.error()));
      return SDL_APP_FAILURE;
    }
    const auto& args = args_opt.value();
    const auto topic = args.getOption<std::string>("topic").value_or("/camera");
    grape::syslog::Note("Subscribing to images on topic: '{}'", topic);

    auto app = std::make_unique<grape::camera::Subscriber>(topic);
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
  auto* app = static_cast<grape::camera::Subscriber*>(appstate);

  static auto last_ts = std::chrono::steady_clock::now();
  const auto now = std::chrono::steady_clock::now();
  static constexpr auto STATS_REPORT_PERIOD = std::chrono::seconds(10);
  const auto dt = now - last_ts;
  if (dt > STATS_REPORT_PERIOD) {
    last_ts = now;
    grape::syslog::Note("Avg. latency={}", app->latency());
  }
  return app->iterate();
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
  auto* app = static_cast<grape::camera::Subscriber*>(appstate);
  return app->handleEvent(event);
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
  // Cleanup is handled automatically by the static app_holder destructor
}
