//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/realtime/fifo_buffer.h"

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit = true;
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    (void)signal(SIGINT, onSignal);
    (void)signal(SIGTERM, onSignal);

    using Fifo = grape::realtime::FIFOBuffer;
    const auto options = Fifo::Options{ .frame_length = 8U, .num_frames = 10U };
    Fifo buffer(options);

    const auto producer = [&buffer](const std::string& name) {
      static constexpr auto UPDATE_PERIOD = std::chrono::milliseconds(100);
      std::uint64_t value = 0;
      while (!s_exit) {
        const auto pushed = buffer.visitToWrite([&value](std::span<std::byte> frame) {
          assert(sizeof(value) == frame.size_bytes());
          std::memcpy(frame.data(), &value, sizeof(value));
        });
        std::this_thread::sleep_for(UPDATE_PERIOD);
        if (pushed) {
          std::println("produce - [{}] {}", name, value);
          ++value;
        } else {
          std::println("produce - [{}] {}", name, "busy");
        }
      }
    };

    const auto consumer = [&buffer]() {
      static constexpr auto REST_PERIOD = std::chrono::seconds(1);
      while (!s_exit) {
        std::uint64_t value{};
        if (buffer.count() > 0) {
          const auto pulled = buffer.visitToRead([&value](std::span<const std::byte> frame) {
            assert(sizeof(value) == frame.size_bytes());
            std::memcpy(&value, frame.data(), frame.size_bytes());
          });
          if (pulled) {
            std::println("consume - {:d}", value);
          } else {
            std::println("consume - empty");
            std::this_thread::sleep_for(REST_PERIOD);
          }
        }
      }
    };

    std::thread t1(producer, "P1");
    std::thread t2(producer, "P2");
    consumer();

    t1.join();
    t2.join();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}