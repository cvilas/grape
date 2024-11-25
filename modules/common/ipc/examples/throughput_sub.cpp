//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <cmath>
#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "throughput_constants.h"

//=================================================================================================
// Subscribing end of the pair of example programs to measure throughput between a publisher and a
// subscriber.
//
// Typical usage:
// ```code
// throughput_sub [--router="proto/address:port"]
// ```
//
// Paired with example: throughput_pub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_sub_thr.cxx
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
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto maybe_args =
        grape::conio::ProgramDescription("Measures throughput from publisher example")
            .declareOption<std::string>("router", "Router locator", "none")
            .parse(argc, argv);
    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    // prepare session
    auto config = grape::ipc::Session::Config{};
    const auto router_str = args.getOptionOrThrow<std::string>("router");
    if (router_str != "none") {
      // If a router is specified, turn this into a client and connect to the router
      config.mode = grape::ipc::Session::Mode::Client;
      config.router = grape::ipc::Locator::fromString(router_str);
      if (not config.router.has_value()) {
        grape::panic<grape::Exception>(std::format("Failed to parse router '{}' ", router_str));
      }
      std::println("Router: '{}'", router_str);
    }
    auto session = grape::ipc::Session(config);

    Statistics stats;

    const auto cb = [&stats](std::span<const std::byte> sample) {
      static constexpr std::array<char, 4> PROGRESS{ '|', '/', '-', '\\' };
      std::print("\r[{}]", PROGRESS.at(stats.msg_count % 4));
      stats.total_bytes += sample.size_bytes();
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

    auto sub = session.createSubscriber(grape::ipc::ex::throughput::TOPIC, cb);

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    std::println("Exiting. Final stats:");
    stats.print();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
