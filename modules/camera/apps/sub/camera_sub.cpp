//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <csignal>
#include <cstring>
#include <mutex>

#include <SDL3/SDL.h>

#include "grape/camera/decompressor.h"
#include "grape/camera/display.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/fifo_buffer.h"
#include "grape/ipc/raw_subscriber.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/wall_clock.h"

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
  void toggleTimestamp();
  [[nodiscard]] auto latency() const -> WallClock::Duration;

private:
  void onReceivedSample(const ipc::Sample& sample);
  void onDecompressedFrame(const ImageFrame& frame);
  void onPublisherMatch(const ipc::Match& match);

  std::atomic_bool mark_fifo_for_deletion_ = false;
  std::unique_ptr<FIFOBuffer> fifo_;
  Display display_;
  Decompressor decompressor_;
  ipc::RawSubscriber subscriber_;
  std::atomic_bool save_snapshot_ = false;
};

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(const std::string& topic)
  : decompressor_([this](const auto& frame) { onDecompressedFrame(frame); })
  , subscriber_(
        topic, ipc::QoS::Reliable, [this](const auto& sample) { onReceivedSample(sample); },
        [this](const auto& match) { onPublisherMatch(match); }) {
}

//-------------------------------------------------------------------------------------------------
void Subscriber::onReceivedSample(const ipc::Sample& sample) {
  if (mark_fifo_for_deletion_) {
    // FIFO is in the process of being deleted. Drop incoming samples until it is reset
    return;
  }

  // create FIFO on first sample
  static constexpr auto SAMPLE_SIZE_OFFSET = sizeof(std::size_t);
  if (not fifo_) {
    static constexpr auto NUM_SAMPLES = 10U;
    const auto max_sample_size = sample.data.size_bytes() * 2U;
    const auto fifo_config = FIFOBuffer::Config{
      .frame_length = SAMPLE_SIZE_OFFSET + max_sample_size,
      .num_frames = NUM_SAMPLES,
    };
    fifo_ = std::make_unique<FIFOBuffer>(fifo_config);
    syslog::Note("FIFO created: {} samples, {} bytes per sample", fifo_config.num_frames,
                 fifo_config.frame_length);
  }

  // define sample writer
  auto is_fifo_frame_size_sufficient = false;
  const auto sample_writer = [&](std::span<std::byte> frame) {
    const auto sample_len = sample.data.size_bytes();
    is_fifo_frame_size_sufficient = (frame.size_bytes() >= SAMPLE_SIZE_OFFSET + sample_len);
    if (not is_fifo_frame_size_sufficient) {
      return;
    }
    std::memcpy(frame.data(), &sample_len, SAMPLE_SIZE_OFFSET);
    std::memcpy(&frame[SAMPLE_SIZE_OFFSET], sample.data.data(), sample_len);
  };

  // Copy sample into FIFO
  const auto write_succeeded = fifo_->visitToWrite(sample_writer);
  if (not is_fifo_frame_size_sufficient) {
    syslog::Error("Frame dropped: FIFO frame size insufficient");
    // Mark current FIFO as invalid. We can't reset and recreate FIFO here as the display thread
    // concurrently accesses it. Mark it as invalid here, so both threads stop interacting with it
    // until the FIFO is reset. In a subsequent call, this method will recreate the FIFO.
    mark_fifo_for_deletion_ = true;
    return;
  }
  if (not write_succeeded) {
    syslog::Error("Frame dropped: FIFO full");
    return;
  }
}

//-------------------------------------------------------------------------------------------------
void Subscriber::update() {
  if (mark_fifo_for_deletion_) {
    fifo_.reset();
    mark_fifo_for_deletion_ = false;
  }

  if (not fifo_) {
    return;
  }

  // define frame reader
  const auto frame_reader = [&](std::span<const std::byte> data) {
    if (data.size_bytes() < sizeof(std::size_t)) {
      return;
    }
    static constexpr auto SAMPLE_SIZE_OFFSET = sizeof(std::size_t);
    std::size_t frame_len = 0;
    std::memcpy(&frame_len, data.data(), SAMPLE_SIZE_OFFSET);
    if (frame_len + SAMPLE_SIZE_OFFSET > data.size_bytes()) {
      return;
    }
    const auto frame_data = data.subspan(SAMPLE_SIZE_OFFSET, frame_len);
    if (not decompressor_.decompress(frame_data)) {
      syslog::Error("Decompression failed!");
    }
  };

  // process all pending frames
  while (fifo_->visitToRead(frame_reader)) {
  }
}

//-------------------------------------------------------------------------------------------------
void Subscriber::onDecompressedFrame(const ImageFrame& frame) {
  static auto last_image_ts = WallClock::TimePoint{};
  const auto image_ts = frame.header.timestamp;
  if (image_ts > last_image_ts) {
    last_image_ts = image_ts;
    display_.render(frame);

    if (save_snapshot_) {
      save_snapshot_ = false;
      const auto fname = std::format("snapshot_{:%FT%T}.bmp", WallClock::now());
      std::ignore = grape::camera::save(frame, fname);
    }
  }
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
void Subscriber::toggleTimestamp() {
  static bool en_ts = false;
  en_ts = !en_ts;
  display_.showTimestamp(en_ts);
}

//-------------------------------------------------------------------------------------------------
auto Subscriber::latency() const -> WallClock::Duration {
  return display_.latency();
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

    if (not SDL_Init(SDL_INIT_VIDEO)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return EXIT_FAILURE;
    }

    // Parse command line arguments
    const auto default_topic = grape::utils::getHostName() + "/camera";
    const auto args = grape::conio::ProgramDescription("Camera viewer application")
                          .declareOption<std::string>("topic", "image stream topic", default_topic)
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
          if (event.key.scancode == SDL_SCANCODE_T) {
            subscriber.toggleTimestamp();
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
