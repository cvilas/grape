//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <print>
#include <thread>
#include <vector>

#include "grape/conio/program_options.h"
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
    static constexpr std::byte DEFAULT_PAYLOAD_FILL{ 0x01 };

    const auto maybe_args =
        grape::conio::ProgramDescription(
            "Publishing end of IPC performance measurement application pair")
            .declareOption<std::size_t>("size", "payload size in bytes", DEFAULT_PAYLOAD_SIZE)
            .parse(argc, argv);
    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();
    const auto payload_size = args.getOption<std::size_t>("size").value();
    const auto payload = std::vector<std::byte>(payload_size, DEFAULT_PAYLOAD_FILL);
    std::println("Payload size: {} bytes", payload_size);

    auto config = grape::ipc::Session::Config{};
    config.scope = grape::ipc::Session::Config::Scope::Network;
    auto session = grape::ipc::Session(config);

    auto pub = session.createPublisher({ .name = grape::ipc::ex::perf::TOPIC });
    std::println("Press CTRL-C to quit");
    while (true) {
      pub.publish(payload);
      // force a context switch to mimic reality of cache invalidation, etc
      std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
