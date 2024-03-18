//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <cmath>
#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Subscribing end of the pair of example programs to measure throughput between a publisher and a
// subscriber.
//
// Paired with example: zenoh_throughput_pub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_sub_thr.c
//=================================================================================================

namespace {

//=================================================================================================
/// Encapsulates data and model for statistics calculations
struct Statistics {
  static constexpr uint64_t REPORT_AT_COUNT = 1000000;
  uint64_t msg_count{ 0 };
  uint64_t total_bytes{ 0 };
  std::chrono::high_resolution_clock::time_point start;
  void print() const;
};

//-------------------------------------------------------------------------------------------------
void Statistics::print() const {
  const auto stop = std::chrono::high_resolution_clock::now();
  const auto dt = std::chrono::duration<double>(stop - start).count();
  const auto msg_rate = std::floor(static_cast<double>(msg_count) / dt);
  const auto byte_rate = std::floor(static_cast<double>(total_bytes) / dt);
  std::println("  {} msg/s [{} bytes/sec]", msg_rate, byte_rate);
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    Statistics stats;

    static constexpr auto TOPIC = "grape/ipc/example/zenoh/throughput";
    const auto cb = [&stats](const zenohc::Sample& sample) {
      static constexpr std::array<char, 4> PROGRESS{ '|', '/', '-', '\\' };
      std::print("\r[{}]", PROGRESS.at(stats.msg_count % 4));
      stats.total_bytes += sample.get_payload().get_len();
      if (stats.msg_count == 0) {
        stats.start = std::chrono::high_resolution_clock::now();
        stats.msg_count++;
      } else if (stats.msg_count < Statistics::REPORT_AT_COUNT) {
        stats.msg_count++;
      } else {
        stats.print();
        stats.msg_count = 0;
        stats.total_bytes = 0;
      }
    };

    auto sub = grape::ipc::expect<zenohc::Subscriber>(session.declare_subscriber(TOPIC, cb));

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    sub.drop();

    std::println("Exiting. Final stats:");
    stats.print();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
