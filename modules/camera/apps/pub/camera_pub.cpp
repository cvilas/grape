//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>

#include <SDL3/SDL.h>

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

  Publisher(const std::string& topic, const std::string& camera_name_hint);
  [[nodiscard]] auto stats() const -> const Stats&;
  void update();

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
void Publisher::update() {
  capture_->acquire();
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

//-------------------------------------------------------------------------------------------------
void setupIpc() {
  auto ipc_config = grape::ipc::Config{};
  ipc_config.scope = grape::ipc::Config::Scope::Network;
  grape::ipc::init(std::move(ipc_config));
}

//-------------------------------------------------------------------------------------------------
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit = true;
  std::puts("\nExit signal received");
}

//-------------------------------------------------------------------------------------------------
void setupSignalHandling() {
  std::ignore = signal(SIGINT, onSignal);
  std::ignore = signal(SIGTERM, onSignal);
}
}  // namespace

//-------------------------------------------------------------------------------------------------
auto main(int argc, char* argv[]) -> int {
  try {
    setupSignalHandling();
    setupLogging();
    setupIpc();

    if (not SDL_Init(SDL_INIT_CAMERA)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return EXIT_FAILURE;
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
      return EXIT_FAILURE;
    }
    const auto& args = args_opt.value();
    const auto camera_name_hint = args.getOption<std::string>("hint").value_or("");
    const auto topic = args.getOption<std::string>("topic").value_or("/camera");

    auto publisher = grape::camera::Publisher(topic, camera_name_hint);
    grape::syslog::Note("Publishing images on topic: '{}'", topic);

    // Main event loop
    SDL_Event event;
    auto last_stats_ts = std::chrono::steady_clock::now();

    while (not s_exit) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          grape::syslog::Info("Quit!");
          return EXIT_SUCCESS;
        }

        if (event.type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
          grape::syslog::Error("Camera access denied!");
          return EXIT_FAILURE;
        }

        if (event.type == SDL_EVENT_CAMERA_DEVICE_APPROVED) {
          grape::syslog::Info("Camera access approved!");
        }
      }

      publisher.update();

      // Periodically report stats
      const auto now = std::chrono::steady_clock::now();
      static constexpr auto STATS_REPORT_PERIOD = std::chrono::seconds(10);
      const auto dt = now - last_stats_ts;
      if (dt > STATS_REPORT_PERIOD) {
        last_stats_ts = now;
        const auto& stats = publisher.stats();
        grape::syslog::Info("Avg. stats: secs/frame={}, bytes/frame={}, compression={}",
                            stats.publish_period.load(std::memory_order_relaxed),
                            stats.publish_bytes.load(std::memory_order_relaxed),
                            stats.format_conv_ratio.load(std::memory_order_relaxed) *
                                stats.compression_ratio.load(std::memory_order_relaxed));
      }
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
