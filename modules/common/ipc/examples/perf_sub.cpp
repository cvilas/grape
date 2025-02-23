//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <cmath>
#include <cstddef>
#include <print>
#include <thread>

#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"
#include "perf_constants.h"

//=================================================================================================
// Subscribing & reporting end of pub/sub pair that measures latency & throughput between endpoints.
//
// Typical usage:
// ```code
// perf_sub
// ```
//
// Paired with example: perf_pub.cpp
//=================================================================================================

//=================================================================================================
/// Encapsulates data and model for statistics calculations
struct Statistics {
  static constexpr auto REPORT_DURATION = std::chrono::seconds(1);
  uint64_t msg_count{ 0 };
  uint64_t aggregate_bytes{ 0 };
  std::chrono::system_clock::duration aggregate_latency{ 0 };
  std::chrono::system_clock::time_point start;
  void print() const;
  void reset();
  void add(const std::chrono::system_clock::time_point& ts, const grape::ipc::Sample& sample);
};

//-------------------------------------------------------------------------------------------------
void Statistics::print() const {
  const auto stop = std::chrono::system_clock::now();
  const auto dt = std::chrono::duration<double>(stop - start).count();
  const auto msg_rate = std::floor(static_cast<double>(msg_count) / dt);
  const auto byte_rate = std::floor(static_cast<double>(aggregate_bytes) / dt);
  const auto avg_latency = aggregate_latency / static_cast<double>(msg_count);
  std::println("  [{} msg/s] [{} bytes/sec] [{} latency]", msg_rate, byte_rate, avg_latency);
}

//-------------------------------------------------------------------------------------------------
void Statistics::reset() {
  msg_count = 0;
  aggregate_bytes = 0;
  aggregate_latency = {};
}

//-------------------------------------------------------------------------------------------------
void Statistics::add(const std::chrono::system_clock::time_point& ts,
                     const grape::ipc::Sample& sample) {
  if (msg_count == 0) {
    start = ts;
  }
  msg_count++;
  aggregate_bytes += sample.data.size_bytes();
  aggregate_latency += (ts - sample.publish_time);
}

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto maybe_args =
        grape::conio::ProgramDescription(
            "Subscribing/reporting end of IPC performance measurement application pair")
            .parse(argc, argv);
    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    [[maybe_unused]] const auto& args = maybe_args.value();
    auto config = grape::ipc::Session::Config{};
    config.scope = grape::ipc::Session::Config::Scope::Network;
    auto session = grape::ipc::Session(config);

    Statistics stats;

    const auto data_cb = [&stats](const grape::ipc::Sample& sample) {
      const auto ts = std::chrono::system_clock::now();
      stats.add(ts, sample);
      static constexpr std::array<char, 4> PROGRESS{ '|', '/', '-', '\\' };
      std::print("\r[{}]", PROGRESS.at(stats.msg_count % 4));

      if (ts - stats.start > Statistics::REPORT_DURATION) {
        stats.print();
        stats.reset();
      }
    };

    auto sub = session.createSubscriber(grape::ipc::ex::perf::TOPIC, data_cb);

    std::println("Press CTRL+C to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (session.ok()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
