//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <csignal>
#include <mutex>

#include <SDL3/SDL.h>

#include "grape/camera/decompressor.h"
#include "grape/camera/display.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/raw_subscriber.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/time.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 to display camera frames acquired by the corresponding publisher appln.
//-------------------------------------------------------------------------------------------------

namespace grape::camera {

//=================================================================================================
/// Encapsulates processing pipeline: [subscribe] -> [decompress] -> [display]
class Subscriber {
public:
  explicit Subscriber(const std::string& topic);

  void update();
  void saveImage();
  [[nodiscard]] auto latency() const -> SystemClock::Duration;

private:
  void onReceivedSample(const ipc::Sample& sample);
  void onDecompressedFrame(const ImageFrame& frame);
  void onPublisherMatch(const ipc::Match& match);

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
auto Subscriber::latency() const -> SystemClock::Duration {
  return display_.latency();
}

//-------------------------------------------------------------------------------------------------
void Subscriber::update() {
  static auto last_image_ts = SystemClock::TimePoint{};

  auto guard = std::lock_guard(image_lock_);
  const auto frame = grape::camera::ImageFrame{ .header = image_header_, .pixels = image_data_ };
  const auto image_ts = frame.header.timestamp;

  if (image_ts > last_image_ts) {
    last_image_ts = image_ts;
    display_.render(frame);

    if (save_snapshot_) {
      save_snapshot_ = false;
      const auto fname = std::format("snapshot_{:%FT%T}.bmp", SystemClock::now());
      std::ignore = grape::camera::save(frame, fname);
    }
  }
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

    if (not SDL_Init(SDL_INIT_VIDEO)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return EXIT_FAILURE;
    }

    // Parse command line arguments
    const auto args = grape::conio::ProgramDescription("Camera viewer application")
                          .declareOption<std::string>("topic", "image stream topic", "/camera")
                          .parse(argc, const_cast<const char**>(argv));

    const auto topic = args.getOption<std::string>("topic");
    grape::syslog::Note("Subscribing to images on topic: '{}'", topic);

    auto subscriber = grape::camera::Subscriber(topic);

    // Main event loop
    SDL_Event event;
    auto last_stats_ts = std::chrono::steady_clock::now();

    while (not s_exit) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
          s_exit = true;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
          if (event.key.scancode == SDL_SCANCODE_S) {
            subscriber.saveImage();
          }
        }
      }

      subscriber.update();

      // Periodically report stats
      const auto now = std::chrono::steady_clock::now();
      static constexpr auto STATS_REPORT_PERIOD = std::chrono::seconds(10);
      const auto dt = now - last_stats_ts;
      if (dt > STATS_REPORT_PERIOD) {
        last_stats_ts = now;
        grape::syslog::Info("Avg. latency={}", subscriber.latency());
      }
    }

    SDL_Quit();
    grape::syslog::Info("Quit!");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
