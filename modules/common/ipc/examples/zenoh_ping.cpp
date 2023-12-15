//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <print>
#include <span>

#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that performs roundtrip time measurements. The ping example performs a put
// operation on a key, waits for a reply from the pong example on a second key and measures the time
// between the two. The pong application waits for samples on the first key expression and replies
// by writing back the received data on the second key expression.
//
// Typical usage:
// ```code
// zenoh_ping [--size=8 --pings=100]
// ```
//
// Paired with example: zenoh_pong.cpp
//
// Derived from https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_ping.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr size_t DEFAULT_PACKET_SIZE = 8;
    static constexpr size_t DEFAULT_NUM_PINGS = 100;

    auto desc = grape::conio::ProgramDescription("Measures roundtrip time with pong program");
    desc.defineOption<size_t>("pings", "number of pings to attempt", DEFAULT_NUM_PINGS)
        .defineOption<size_t>("size", "ping and pong payload size in bytes", DEFAULT_PACKET_SIZE);

    const auto args = std::move(desc).parse(argc, argv);
    const auto num_pings = args.getOption<size_t>("pings");
    const auto size = args.getOption<size_t>("size");
    std::println("Payload size: {}", size);
    std::println("Number of pings: {}", num_pings);

    // prepare session
    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    // prepare publisher
    static constexpr auto PING_KEY = "grape/ipc/example/zenoh/ping";
    auto pub = grape::ipc::expect<zenohc::Publisher>(session.declare_publisher(PING_KEY));

    std::mutex pong_mut;
    std::condition_variable pong_cond;
    bool pong_received = false;

    // prepare subscriber
    static constexpr auto PONG_KEY = "grape/ipc/example/zenoh/pong";
    const auto cb = [&pong_mut, &pong_cond, &pong_received](const zenohc::Sample& sample) {
      const auto payload = sample.sample_payload_rcinc();
      std::print("Pong [{} bytes] ", payload.get_payload().get_len());
      std::lock_guard lk(pong_mut);
      pong_received = true;
      pong_cond.notify_one();
    };
    auto sub = grape::ipc::expect<zenohc::Subscriber>(session.declare_subscriber(PONG_KEY, cb));

    // ping-pong
    static constexpr auto PONG_WAIT_TIMEOUT = std::chrono::milliseconds(10000);
    const auto data = std::vector<uint8_t>(size);
    auto results = std::vector<std::chrono::high_resolution_clock::duration>(num_pings);
    for (size_t i = 0; i < num_pings; i++) {
      const auto ts_start = std::chrono::high_resolution_clock::now();
      pub.put(data);
      std::print("Ping ");
      std::unique_lock lk(pong_mut);
      if (pong_cond.wait_for(lk, PONG_WAIT_TIMEOUT, [&pong_received] { return pong_received; })) {
        const auto ts_stop = std::chrono::high_resolution_clock::now();
        const auto rtt = ts_stop - ts_start;
        std::println("seq={} rtt={}, lat={}", i, rtt, rtt / 2);
        results[i] = rtt;
        pong_received = false;
      } else {
        std::println("Timed out waiting for response. Exiting");
        return EXIT_SUCCESS;
      }
    }

    // print statistics
    auto min_rtt = std::chrono::high_resolution_clock::duration::max();
    auto max_rtt = std::chrono::high_resolution_clock::duration::min();
    auto avg_rtt = std::chrono::high_resolution_clock::duration::zero();
    for (auto& r : results) {
      if (r < min_rtt) {
        min_rtt = r;
      }
      if (r > max_rtt) {
        max_rtt = r;
      }
      avg_rtt += r;
    }
    avg_rtt = avg_rtt / static_cast<int>(results.size());
    std::println("Ping Pong rtt stats ({} pings): min={}, avg={}, max={}", num_pings, min_rtt,
                 avg_rtt, max_rtt);
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
