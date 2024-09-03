//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <chrono>
#include <csignal>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that performs roundtrip time measurements. The ping example performs a put
// operation on a key, waits for a reply from the pong example on a second key and measures the time
// between the two. The pong application waits for samples on the first key expression and replies
// by writing back the received data on the second key expression.
//
// Paired with example: zenoh_ping.cpp
//
// Derived from https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_pong.cxx
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//-------------------------------------------------------------------------------------------------
auto main() -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));
    static constexpr auto PING_KEY = "grape/ipc/example/zenoh/ping";
    static constexpr auto PONG_KEY = "grape/ipc/example/zenoh/pong";

    auto pub = session.declare_publisher(PONG_KEY);

    const auto cb = [&pub](const zenoh::Sample& sample) {
      const auto& payload = sample.get_payload();
      const auto payload_len = payload.size();
      pub.put(payload.clone());
      std::println("pong [{} bytes]", payload_len);
    };

    [[maybe_unused]] auto sub = session.declare_subscriber(PING_KEY, cb, zenoh::closures::none);

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);
    return EXIT_SUCCESS;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}