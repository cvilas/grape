//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <chrono>
#include <csignal>
#include <print>
#include <thread>

#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that performs roundtrip time measurements. The ping example performs a put
// operation on a key, waits for a reply from the pong example on a second key and measures the time
// between the two. The pong application waits for samples on the first key expression and replies
// by writing back the received data on the second key expression.
//
// Paired with example: zenoh_ping.cpp
//
// Derived from https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_pong.c
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int signum) {
  std::println("\nReceived signal {}", strsignal(signum));
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//-------------------------------------------------------------------------------------------------
auto main() -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    zenohc::Config config;
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));
    static constexpr auto PING_KEY = "grape/ipc/example/zenoh/ping";
    static constexpr auto PONG_KEY = "grape/ipc/example/zenoh/pong";

    auto pub = grape::ipc::expect<zenohc::Publisher>(session.declare_publisher(PONG_KEY));

    const auto cb = [&pub](const zenohc::Sample& sample) {
      auto payload = sample.sample_payload_rcinc();
      const auto payload_len = payload.get_payload().get_len();
      pub.put_owned(std::move(payload));
      std::println("pong [{} bytes]", payload_len);
    };

    auto sub = grape::ipc::expect<zenohc::Subscriber>(session.declare_subscriber(PING_KEY, cb));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}