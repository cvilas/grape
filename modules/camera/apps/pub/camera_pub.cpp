//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <atomic>
#include <chrono>

#include <SDL3/SDL_main.h>

#include "grape/camera/camera.h"
#include "grape/camera/compressor.h"
#include "grape/camera/formatter.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/statistics/sliding_mean.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 camera API to acquire, reformat, compress and publish camera frames.
// Set SDL_CAMERA_DRIVER environment variable to specify a driver backend.
// Eg: SDL_CAMERA_DRIVER=v4l2 grape_camera_pub
//-------------------------------------------------------------------------------------------------

namespace grape::camera {

//=================================================================================================
/// Encapsulates processing pipeline: [capture] -> [format] -> [compress] -> [publish]
class Publisher {
public:
  struct Stats {
    std::atomic<float> format_conv_ratio;
    std::atomic<float> compression_ratio;
    std::atomic<float> publish_period;
    std::atomic<std::size_t> publish_bytes;
  };

  explicit Publisher(const std::string& topic, const std::string& camera_name_hint);
  [[nodiscard]] auto stats() const -> const Stats&;
  auto iterate() -> SDL_AppResult;
  auto handleEvent(SDL_Event* event) -> SDL_AppResult;

private:
  void onCapturedFrame(const ImageFrame& frame);
  void onFormattedFrame(const ImageFrame& frame, const Formatter::Stats& fmt_stats);
  void onCompressedFrame(std::span<const std::byte> bytes, const Compressor::Stats& comp_stats);
  void onSubscriberMatch(const ipc::Match& match);

  static constexpr auto STATS_WINDOW = 600U;
  Stats stats_;
  statistics::SlidingMean<float, STATS_WINDOW> publish_period_;
  statistics::SlidingMean<float, STATS_WINDOW> publish_bytes_;
  statistics::SlidingMean<float, STATS_WINDOW> compression_stats_;
  statistics::SlidingMean<float, STATS_WINDOW> formatter_stats_;

  ipc::RawPublisher publisher_;
  Compressor compressor_;
  Formatter formatter_;
  std::unique_ptr<Camera> capture_;
};

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(const std::string& topic, const std::string& camera_name_hint)
  : publisher_(topic, [this](const auto& match) { onSubscriberMatch(match); })
  , compressor_([this](const auto& frame, const auto& stats) { onCompressedFrame(frame, stats); })
  , formatter_([this](const auto& frame, const auto& stats) { onFormattedFrame(frame, stats); })
  , capture_(std::make_unique<Camera>([this](const auto& frame) { onCapturedFrame(frame); },
                                      camera_name_hint)) {
}

//-------------------------------------------------------------------------------------------------
void Publisher::onCapturedFrame(const ImageFrame& frame) {
  if (not formatter_.format(frame)) {
    syslog::Error("Formatting failed!");
  }
}

//-------------------------------------------------------------------------------------------------
void Publisher::onFormattedFrame(const ImageFrame& frame, const Formatter::Stats& fmt_stats) {
  const auto stats = formatter_stats_.append(static_cast<float>(fmt_stats.src_size) /
                                             static_cast<float>(fmt_stats.dst_size));
  stats_.format_conv_ratio.store(stats.mean, std::memory_order_relaxed);
  if (not compressor_.compress(frame)) {
    syslog::Error("Compression failed!");
  }
}

//-------------------------------------------------------------------------------------------------
void Publisher::onCompressedFrame(std::span<const std::byte> bytes,
                                  const Compressor::Stats& comp_stats) {
  {
    const auto stats = compression_stats_.append(static_cast<float>(comp_stats.src_size) /
                                                 static_cast<float>(comp_stats.dst_size));
    stats_.compression_ratio.store(stats.mean, std::memory_order_relaxed);
  }
  {
    const auto stats = publish_bytes_.append(static_cast<float>(bytes.size_bytes()));
    stats_.publish_bytes.store(static_cast<std::size_t>(stats.mean), std::memory_order_relaxed);
  }

  publisher_.publish(bytes);

  static auto last_ts = std::chrono::steady_clock::now();
  auto ts = std::chrono::steady_clock::now();
  const auto dt = std::chrono::duration_cast<std::chrono::duration<float>>(ts - last_ts).count();
  last_ts = ts;
  {
    const auto stats = publish_period_.append(dt);
    stats_.publish_period.store(stats.mean, std::memory_order_relaxed);
  }
}

//-------------------------------------------------------------------------------------------------
void Publisher::onSubscriberMatch(const ipc::Match& match) {
  (void)this;
  syslog::Note("{} (entity: {})", toString(match.status), toString(match.remote_entity));
}

//-------------------------------------------------------------------------------------------------
auto Publisher::stats() const -> const Stats& {
  return stats_;
}

//-------------------------------------------------------------------------------------------------
auto Publisher::iterate() -> SDL_AppResult {
  capture_->acquire();
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto Publisher::handleEvent(SDL_Event* event) -> SDL_AppResult {
  if (event->type == SDL_EVENT_QUIT) {
    syslog::Info("Quit!");
    capture_.reset();
    return SDL_APP_SUCCESS;
  }

  if (event->type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
    syslog::Error("Camera access denied!");
    return SDL_APP_FAILURE;
  }

  if (event->type == SDL_EVENT_CAMERA_DEVICE_APPROVED) {
    syslog::Info("Camera access approved!");
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

    if (not SDL_Init(SDL_INIT_CAMERA)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    // Parse command line arguments
    const auto args_opt =
        grape::conio::ProgramDescription("Camera viewer application")
            .declareOption<std::string>("hint", "Part of camera name to match", "")
            .declareOption<std::string>("topic", "image stream topic", "/camera")
            .parse(argc, const_cast<const char**>(argv));

    if (not args_opt.has_value()) {
      grape::syslog::Critical("Failed to parse command line arguments: {}",
                              toString(args_opt.error()));
      return SDL_APP_FAILURE;
    }
    const auto& args = args_opt.value();
    const auto camera_name_hint = args.getOption<std::string>("hint").value_or("");
    const auto topic = args.getOption<std::string>("topic").value_or("/camera");

    auto app = std::make_unique<grape::camera::Publisher>(topic, camera_name_hint);
    *appstate = app.get();
    grape::syslog::Note("Publishing images on topic: '{}'", topic);

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
  auto* app = static_cast<grape::camera::Publisher*>(appstate);

  static auto last_ts = std::chrono::steady_clock::now();
  const auto now = std::chrono::steady_clock::now();
  static constexpr auto STATS_REPORT_PERIOD = std::chrono::seconds(10);
  const auto dt = now - last_ts;
  if (dt > STATS_REPORT_PERIOD) {
    last_ts = now;
    const auto& stats = app->stats();
    grape::syslog::Info("Avg. stats: secs/frame={}, bytes/frame={}, compression={}",
                        stats.publish_period.load(std::memory_order_relaxed),
                        stats.publish_bytes.load(std::memory_order_relaxed),
                        stats.format_conv_ratio.load(std::memory_order_relaxed) *
                            stats.compression_ratio.load(std::memory_order_relaxed));
  }
  return app->iterate();
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
  auto* app = static_cast<grape::camera::Publisher*>(appstate);
  return app->handleEvent(event);
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
  // Cleanup is handled automatically by the static app_holder destructor
}
