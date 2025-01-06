//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <print>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/common.h"
#include "grape/ipc/session.h"
#include "ping_pong_constants.h"

//=================================================================================================
// Example program that performs roundtrip time measurements. The ping example performs a put
// operation on a key, waits for a reply from the pong example on a second key and measures the time
// between the two. The pong application waits for samples on the first key expression and replies
// by writing back the received data on the second key expression.
//
// Typical usage:
// ```code
// ping [--size=8 --pings=100 --router="proto/address:port"]
// ```
//
// Paired with example: pong.cpp
//
// Derived from https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_ping.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr size_t DEFAULT_PACKET_SIZE = 8;
    static constexpr size_t DEFAULT_NUM_PINGS = 100;
    static constexpr auto WARMUP_PINGS = 100U;

    const auto maybe_args =
        grape::conio::ProgramDescription("Measures roundtrip time with pong program")
            .declareOption<std::size_t>("pings", "Number of pings to attempt", DEFAULT_NUM_PINGS)
            .declareOption<std::size_t>("size", "Payload size in bytes", DEFAULT_PACKET_SIZE)
            .declareOption<std::string>("router", "Router locator", "none")
            .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();
    const auto num_pings = args.getOptionOrThrow<std::size_t>("pings");
    const auto size = args.getOptionOrThrow<std::size_t>("size");

    std::println("Payload size: {}", size);
    std::println("Number of pings: {}", num_pings);

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

    // prepare publisher
    auto pub = session.createPublisher({ .key = grape::ipc::ex::ping::PING_TOPIC });

    std::mutex pong_mut;
    std::condition_variable pong_cond;
    bool pong_received = false;

    // prepare subscriber
    const auto cb = [&pong_mut, &pong_cond, &pong_received](std::span<const std::byte> bytes) {
      std::print("Pong [{} bytes] ", bytes.size_bytes());
      const std::lock_guard lk(pong_mut);
      pong_received = true;
      pong_cond.notify_one();
    };
    auto sub = session.createSubscriber(grape::ipc::ex::ping::PONG_TOPIC, cb);

    // ping-pong
    static constexpr auto PONG_WAIT_TIMEOUT = std::chrono::milliseconds(10000);
    const auto data = std::vector<std::byte>(size);
    const auto total_pings = num_pings + WARMUP_PINGS;
    auto round_trip_durations = std::vector<std::chrono::high_resolution_clock::duration>{};
    round_trip_durations.reserve(total_pings);
    for (std::size_t i = 0; i < total_pings; i++) {
      const auto ts_start = std::chrono::high_resolution_clock::now();
      pub.publish(data);
      std::print("Ping ");
      std::unique_lock lk(pong_mut);
      if (pong_cond.wait_for(lk, PONG_WAIT_TIMEOUT, [&pong_received] { return pong_received; })) {
        const auto ts_stop = std::chrono::high_resolution_clock::now();
        const auto& rtt = round_trip_durations.emplace_back(ts_stop - ts_start);
        std::println("seq={}, rtt={}", i, rtt);
        pong_received = false;
      } else {
        std::println("Timed out waiting for response. Exiting");
        return EXIT_SUCCESS;
      }
    }

    // remove warmup runs
    round_trip_durations.erase(round_trip_durations.begin(),
                               round_trip_durations.begin() + WARMUP_PINGS);

    // print statistics
    auto rtt_min = std::chrono::high_resolution_clock::duration::max();
    auto rtt_max = std::chrono::high_resolution_clock::duration::min();
    auto rtt_avg = std::chrono::high_resolution_clock::duration::zero();
    for (auto& rtt : round_trip_durations) {
      if (rtt < rtt_min) {
        rtt_min = rtt;
      }
      if (rtt > rtt_max) {
        rtt_max = rtt;
      }
      rtt_avg += rtt;
    }
    rtt_avg = rtt_avg / static_cast<int>(round_trip_durations.size());
    std::println("{} pings ({} bytes): rtt_min={}, rtt_avg={}, rtt_max={}", num_pings, size,
                 rtt_min, rtt_avg, rtt_max);
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
