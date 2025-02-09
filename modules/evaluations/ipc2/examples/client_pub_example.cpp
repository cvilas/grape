//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <cstdint>
#include <print>
#include <thread>

#include "client_example_constants.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc2/common.h"
#include "grape/ipc2/session.h"
#include "grape/utils/ip.h"

//=================================================================================================
// Example program that creates a 'client' publisher. Clients connect to other peers and clients
// via routers. For a description of clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/. This publisher periodically writes a value on
// the specified key. The published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// pub_client_example [--router="proto/address:port"]
// ```
//
// Paired with: sub_client.cpp, router.cpp
//
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit = true;
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    static constexpr auto DEFAULT_MSG = "Hi from publishing client";

    const auto maybe_args =
        grape::conio::ProgramDescription("Example publisher operating in 'client' mode")
            .declareOption<std::string>("router", "Router locator",
                                        grape::ipc2::ex::client::DEFAULT_ROUTER)
            .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    // configure as client to the specified router
    auto config = grape::ipc2::Session::Config{};
    config.mode = grape::ipc2::Session::Mode::Client;
    const auto router_str = args.getOptionOrThrow<std::string>("router");
    if (router_str != "none") {
      config.router = grape::ipc2::Locator::fromString(router_str);
      if (not config.router.has_value()) {
        grape::panic<grape::Exception>(std::format("Failed to parse router '{}' ", router_str));
      }
      std::println("Router: '{}'", router_str);
    }
    auto session = grape::ipc2::Session(config);

    auto pub = session.createPublisher({ .key = grape::ipc2::ex::client::TOPIC });
    std::println("Publisher created on '{}'", grape::ipc2::ex::client::TOPIC);

    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    std::println("Press ctrl-c to exit");
    while (not s_exit) {
      const auto msg = std::format("[{}] {}", idx++, DEFAULT_MSG);
      std::println("Publishing message: '{}'", msg);
      pub.publish({ msg.data(), msg.size() });
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
