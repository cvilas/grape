//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>

#include "grape/camera/compressor.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/rpi/camera.h"
#include "grape/script/script.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using libcamera to acquire, compress and publish camera frames.
//-------------------------------------------------------------------------------------------------

namespace grape::rpi::camera {

//=================================================================================================
/// Encapsulates processing pipeline:
/// [capture] -> [compress] -> [publish]
class Publisher {
public:
  struct Config {
    Camera::Config camera_config;
    std::string pub_topic{ "/picam" };
    std::uint16_t compression_speed{ 1 };
    static auto init(const grape::script::ConfigTable& table) -> Config;
  };

  explicit Publisher(const Config& cfg);
  void update();

private:
  void onCapturedFrame(const grape::camera::ImageFrame& frame);
  void onCompressedFrame(std::span<const std::byte> bytes,
                         const grape::camera::Compressor::Stats& stats);
  void onSubscriberMatch(const ipc::Match& match);

  ipc::RawPublisher publisher_;
  grape::camera::Compressor compressor_;
  std::unique_ptr<rpi::Camera> capture_;
};

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(const Config& cfg)
  : publisher_(cfg.pub_topic, [this](const auto& match) { onSubscriberMatch(match); })
  , compressor_(cfg.compression_speed,
                [this](const auto& frame, const auto& stats) { onCompressedFrame(frame, stats); })
  , capture_(std::make_unique<rpi::Camera>(cfg.camera_config,
                                           [this](const auto& frame) { onCapturedFrame(frame); })) {
  syslog::Note("Publishing images on topic: '{}'", cfg.pub_topic);
}

//-------------------------------------------------------------------------------------------------
void Publisher::onCapturedFrame(const grape::camera::ImageFrame& frame) {
  if (not compressor_.compress(frame)) {
    syslog::Error("Compression failed!");
  }
}

//-------------------------------------------------------------------------------------------------
void Publisher::onCompressedFrame(std::span<const std::byte> bytes,
                                  const grape::camera::Compressor::Stats& stats) {
  const auto pub_result = publisher_.publish(bytes);
  if (not pub_result) {
    syslog::Error("Publish failed: {}", toString(pub_result.error()));
  }

  auto ts = std::chrono::steady_clock::now();
  static auto last_ts = ts;
  const auto dt = ts - last_ts;
  last_ts = ts;

  const auto comp_ratio = static_cast<float>(stats.src_size) / static_cast<float>(stats.dst_size);
  syslog::Debug("dt={}, bytes={}, comp_ratio={}", dt, bytes.size_bytes(), comp_ratio);
}

//-------------------------------------------------------------------------------------------------
void Publisher::onSubscriberMatch(const ipc::Match& match) {
  (void)this;
  syslog::Note("{} (entity: {})", toString(match.status), toString(match.remote_entity));
}

//-------------------------------------------------------------------------------------------------
void Publisher::update() {
  capture_->acquire();
}

//-------------------------------------------------------------------------------------------------
auto Publisher::Config::init(const script::ConfigTable& table) -> Publisher::Config {
  return Config{
    .camera_config{
        .camera_name_hint = table.readOrThrow<std::string>("camera_name"),
        .image_size{
            .width = static_cast<std::uint16_t>(table.readOrThrow<int>("image_width")),   //
            .height = static_cast<std::uint16_t>(table.readOrThrow<int>("image_height"))  //
        } },
    .pub_topic = table.readOrThrow<std::string>("pub_topic"),
    .compression_speed = static_cast<std::uint16_t>(table.readOrThrow<int>("compression_speed"))
  };
}

}  // namespace grape::rpi::camera

namespace {
//-------------------------------------------------------------------------------------------------
struct LogFormatter {
  static auto format(const grape::log::Record& record) -> std::string {
    return std::format("[{}] [{:9s}] {}", record.timestamp, toString(record.severity),
                       record.message.cStr());
  }
};

//-------------------------------------------------------------------------------------------------
void setupLogging(grape::log::Severity sev) {
  auto log_config = grape::log::Config{};
  log_config.sink = std::make_shared<grape::log::ConsoleSink<LogFormatter>>();
  log_config.threshold = sev;
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
    // Parse command line arguments
    const auto args =
        grape::conio::ProgramDescription("Raspberry-pi camera publisher application")
            .declareOption<std::string>("config", "Configuration file", "rpi_camera_pub/config.lua")
            .declareOption<std::string>("log_level", "Log severity level", "Info")
            .parse(argc, const_cast<const char**>(argv));

    const auto& maybe_log_level =
        grape::enums::cast<grape::log::Severity>(args.getOption<std::string>("log_level"));
    const auto log_level = maybe_log_level ? maybe_log_level.value() : grape::log::Severity::Debug;

    setupSignalHandling();
    setupLogging(log_level);
    setupIpc();

    const auto config_file_name = args.getOption<std::string>("config");
    const auto config_file_path = grape::utils::resolveFilePath(config_file_name);
    if (not config_file_path) {
      grape::syslog::Critical("Could not find config file '{}'", config_file_name);
      return EXIT_FAILURE;
    }
    grape::syslog::Note("Using config file '{}'", config_file_path.value().string());
    const auto config_script = grape::script::ConfigScript(config_file_path.value());
    const auto config = grape::rpi::camera::Publisher::Config::init(config_script.table());

    auto publisher = std::make_unique<grape::rpi::camera::Publisher>(config);
    while (not s_exit) {
      publisher->update();
    }
    publisher.reset();
    grape::syslog::Info("Quit!");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
