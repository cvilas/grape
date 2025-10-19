//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc2/session.h"
#include "ping_pong_constants.h"

//=================================================================================================
// Example program that performs roundtrip time measurements. The ping example performs a put
// operation on a key, waits for a reply from the pong example on a second key and measures the time
// between the two. The pong application waits for samples on the first key expression and replies
// by writing back the received data on the second key expression.
//
// Typical usage:
// ```code
// pong [--router="proto/address:port"]
// ```
//
// Paired with example: ping.cpp
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
auto main(int argc, const char* argv[]) -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    const auto maybe_args = grape::conio::ProgramDescription("Responds to ping program")
                                .declareOption<std::string>("router", "Router locator", "none")
                                .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    // prepare session
    auto config = grape::ipc2::Session::Config{};
    const auto router_str = args.getOptionOrThrow<std::string>("router");
    if (router_str != "none") {
      // If a router is specified, turn this into a client and connect to the router
      config.mode = grape::ipc2::Session::Mode::Client;
      config.router = grape::ipc2::Locator::fromString(router_str);
      if (not config.router.has_value()) {
        grape::panic<grape::Exception>(std::format("Failed to parse router '{}' ", router_str));
      }
      std::println("Router: '{}'", router_str);
    }
    auto session = grape::ipc2::Session(config);
    auto pub = session.createPublisher({ .key = grape::ipc2::ex::ping::PONG_TOPIC });

    const auto cb = [&pub](const std::span<const std::byte> bytes) {
      const auto payload_len = bytes.size_bytes();
      pub.publish(bytes);
      std::println("pong [{} bytes]", payload_len);
    };

    auto sub = session.createSubscriber(grape::ipc2::ex::ping::PING_TOPIC, cb);

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
