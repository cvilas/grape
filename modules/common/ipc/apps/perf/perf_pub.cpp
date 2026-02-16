//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>
#include <vector>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/session.h"
#include "perf_constants.h"

//=================================================================================================
// Publishing end of pub/sub pair that measures latency and throughput between endpoints.
//
// Run perf_sub to get the stats
//
// Typical usage:
// ```code
// perf_pub [--size=8]
// ```
//
// Paired with example: perf_sub.cpp
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_PAYLOAD_SIZE = 8U;
    static constexpr auto DEFAULT_INTERVAL = 0.0;
    static constexpr std::byte DEFAULT_PAYLOAD_FILL{ 0x01 };

    const auto args =
        grape::conio::ProgramDescription(
            "Publishing end of IPC performance measurement application pair")
            .declareOption<std::size_t>("size", "payload size in bytes", DEFAULT_PAYLOAD_SIZE)
            .declareOption<double>("interval", "publish interval in seconds", DEFAULT_INTERVAL)
            .parse(argc, argv);

    const auto payload_size = args.getOption<std::size_t>("size");
    const auto payload = std::vector<std::byte>(payload_size, DEFAULT_PAYLOAD_FILL);
    std::println("Payload size: {} bytes", payload_size);

    const auto interval_sec = std::chrono::duration<double>(args.getOption<double>("interval"));
    const auto interval =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(interval_sec);
    std::println("Publish interval: {} s", interval_sec.count());

    const auto config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
    grape::ipc::init(config);

    auto pub = grape::ipc::RawPublisher(grape::ipc::ex::perf::TOPIC);
    auto next_tick = std::chrono::steady_clock::now();
    std::println("Press CTRL-C to quit");
    while (grape::ipc::ok()) {
      const auto pub_result = pub.publish(payload);
      if (not pub_result) {
        std::println("ERROR: {}", toString(pub_result.error()));
      }
      next_tick += interval;
      std::this_thread::sleep_until(next_tick);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
