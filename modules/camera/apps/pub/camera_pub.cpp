//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>

#include <SDL3/SDL.h>

#include "grape/camera/camera.h"
#include "grape/camera/compressor.h"
#include "grape/camera/formatter.h"
#include "grape/camera/rate_limiter.h"
#include "grape/camera/scaler.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/script/script.h"
#include "grape/statistics/sliding_mean.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 camera API to acquire, reformat, compress and publish camera frames.
// Set SDL_CAMERA_DRIVER environment variable to specify a driver backend.
// Eg: SDL_CAMERA_DRIVER=v4l2 grape_camera_pub
//-------------------------------------------------------------------------------------------------

namespace grape::camera {

//=================================================================================================
/// Encapsulates processing pipeline:
/// [capture] -> [rate limit] -> [scale] -> [format] -> [compress] -> [publish]
class Publisher {
public:
  struct Stats {
    std::atomic<float> format_conv_ratio;
    std::atomic<float> compression_ratio;
    std::atomic<float> publish_period;
    std::atomic<std::size_t> publish_bytes;
  };

  struct Config {
    std::string camera_name;
    std::string pub_topic{ "/camera" };
    float image_scale_factor{ 1.F };
    std::uint8_t frame_rate_divisor{ 1U };
    std::uint16_t compression_speed{ 1 };
    static auto init(const grape::script::ConfigTable& table) -> Config;
  };

  explicit Publisher(const Config& cfg);
  [[nodiscard]] auto stats() const -> const Stats&;
  void update();

private:
  void onCapturedFrame(const ImageFrame& frame);
  void onScaledFrame(const ImageFrame& frame);
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
  Scaler scaler_;
  RateLimiter rate_limiter_;
  std::unique_ptr<Camera> capture_;
};

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(const Config& cfg)
  : publisher_(cfg.pub_topic, [this](const auto& match) { onSubscriberMatch(match); })
  , compressor_(cfg.compression_speed,
                [this](const auto& frame, const auto& stats) { onCompressedFrame(frame, stats); })
  , formatter_([this](const auto& frame, const auto& stats) { onFormattedFrame(frame, stats); })
  , scaler_(cfg.image_scale_factor, [this](const auto& frame) { onScaledFrame(frame); })
  , rate_limiter_(cfg.frame_rate_divisor, [this](const auto& frame) { onCapturedFrame(frame); })
  , capture_(std::make_unique<Camera>([this](const auto& frame) { rate_limiter_.process(frame); },
                                      cfg.camera_name)) {
  syslog::Note("Publishing images on topic: '{}'", cfg.pub_topic);
}

//-------------------------------------------------------------------------------------------------
void Publisher::onCapturedFrame(const ImageFrame& frame) {
  if (not scaler_.scale(frame)) {
    syslog::Error("Scaling failed!");
  }
}

//-------------------------------------------------------------------------------------------------
void Publisher::onScaledFrame(const ImageFrame& frame) {
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

  const auto pub_result = publisher_.publish(bytes);
  if (not pub_result) {
    syslog::Error("Publish failed: {}", toString(pub_result.error()));
  }

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

//-------------------------------------------------------------------------------------------------
auto Publisher::Config::init(const script::ConfigTable& table) -> Publisher::Config {
  return Config{
    //
    .camera_name = table.readOrThrow<std::string>("camera_name"),
    .pub_topic = table.readOrThrow<std::string>("pub_topic"),
    .image_scale_factor = table.readOrThrow<float>("image_scale_factor"),
    .frame_rate_divisor = static_cast<std::uint8_t>(table.readOrThrow<int>("frame_rate_divisor")),
    .compression_speed = static_cast<std::uint16_t>(table.readOrThrow<int>("compression_speed"))
  };
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
  const auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
  grape::ipc::init(ipc_config);
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
    const auto args =
        grape::conio::ProgramDescription("Camera viewer application")
            .declareOption<std::string>("config", "Configuration file", "camera_pub/config.lua")
            .parse(argc, const_cast<const char**>(argv));
    const auto config_file_name = args.getOption<std::string>("config");
    const auto config_file_path = grape::utils::resolveFilePath(config_file_name);
    if (not config_file_path) {
      grape::syslog::Critical("Could not find config file '{}'", config_file_name);
      return EXIT_FAILURE;
    }
    grape::syslog::Note("Using config file '{}'", config_file_path.value().string());
    const auto config_script = grape::script::ConfigScript(config_file_path.value());
    const auto config = grape::camera::Publisher::Config::init(config_script.table());

    auto publisher = std::make_unique<grape::camera::Publisher>(config);

    // Main event loop
    SDL_Event event;
    auto last_stats_ts = std::chrono::steady_clock::now();

    while (not s_exit) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          s_exit = true;
        }

        if (event.type == SDL_EVENT_CAMERA_DEVICE_DENIED) {
          grape::syslog::Error("Camera access denied!");
          return EXIT_FAILURE;
        }

        if (event.type == SDL_EVENT_CAMERA_DEVICE_APPROVED) {
          grape::syslog::Info("Camera access approved!");
        }
      }

      publisher->update();

      // Periodically report stats
      const auto now = std::chrono::steady_clock::now();
      static constexpr auto STATS_REPORT_PERIOD = std::chrono::seconds(10);
      const auto dt = now - last_stats_ts;
      if (dt > STATS_REPORT_PERIOD) {
        last_stats_ts = now;
        const auto& stats = publisher->stats();
        grape::syslog::Info("Avg. stats: secs/frame={}, bytes/frame={}, compression={}",
                            stats.publish_period.load(std::memory_order_relaxed),
                            stats.publish_bytes.load(std::memory_order_relaxed),
                            stats.format_conv_ratio.load(std::memory_order_relaxed) *
                                stats.compression_ratio.load(std::memory_order_relaxed));
      }
    }
    publisher.reset();
    SDL_Quit();
    grape::syslog::Info("Quit!");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
