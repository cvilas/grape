//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <cstring>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/realtime/spmc_ring_buffer.h"

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    (void)signal(SIGINT, onSignal);
    (void)signal(SIGTERM, onSignal);

    static constexpr auto NAME = "spmc_example";

    const auto producer = [](const std::stop_token& st) {
      // create
      const auto config = grape::spmc_ring_buffer::Config{ .frame_length = 8U, .num_frames = 10U };
      auto buffer = grape::spmc_ring_buffer::Writer::create(NAME, config);
      if (not buffer) {
        std::println("'{}' - {}", NAME, buffer.error().message());
        return;
      }

      // write
      static constexpr auto UPDATE_PERIOD = std::chrono::milliseconds(100);
      std::uint64_t value = 0;
      while (not st.stop_requested()) {
        buffer->visit([&value](std::span<std::byte> frame) {
          std::memcpy(frame.data(), &value, sizeof(value));
        });
        value++;
        std::this_thread::sleep_for(UPDATE_PERIOD);
      }
    };

    auto consumer_count = std::atomic_int{};
    const auto consumer = [&consumer_count](const std::stop_token& st) {
      const auto consumer_id = consumer_count.fetch_add(1);

      // wait for writer
      static constexpr auto PRODUCER_WAIT_PERIOD = std::chrono::milliseconds(1000);
      while (not(st.stop_requested() or grape::spmc_ring_buffer::Reader::exists(NAME))) {
        std::println("consumer {}: Waiting for producer..", consumer_id);
        std::this_thread::sleep_for(PRODUCER_WAIT_PERIOD);
      }

      // connect
      auto buffer = grape::spmc_ring_buffer::Reader::connect(NAME);
      if (not buffer) {
        std::println("consumer {}: '{}' - {}", consumer_id, NAME, buffer.error().message());
        return;
      }

      // read
      static constexpr auto REST_PERIOD = std::chrono::milliseconds(200);
      while (not st.stop_requested()) {
        std::uint64_t value{};
        const auto status = buffer->visit([&value](std::span<const std::byte> frame) {
          std::memcpy(&value, frame.data(), frame.size_bytes());
        });
        std::println("consumer {} {} - {:d}", consumer_id, toString(status), value);
        std::this_thread::sleep_for(REST_PERIOD);
      }
    };

    const auto t1 = std::jthread(producer);
    const auto t2 = std::jthread(consumer);
    const auto t3 = std::jthread(consumer);

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
