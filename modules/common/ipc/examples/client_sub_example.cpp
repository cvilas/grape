//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "client_example_constants.h"
#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"

//=================================================================================================
// Example program that creates a 'client' subscriber. Clients connect to other peers and clients
// via routers. For a description of clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/.
//
// Typical usage:
// ```bash
// sub_client_example [--router="proto/address:port"]
// ```
//
// Paired with: pub_client.cpp, router.cpp
//
//=================================================================================================

namespace {

std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

auto asString(std::span<const std::byte> bytes) -> std::string_view {
  return { // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
           reinterpret_cast<const char*>(bytes.data()),  //
           bytes.size_bytes()
  };
};

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    const auto maybe_args =
        grape::conio::ProgramDescription("Example subscriber operating in 'client' mode")
            .declareOption<std::string>("router", "Router locator",
                                        grape::ipc::ex::client::DEFAULT_ROUTER)
            .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    // configure as client to the specified router
    auto config = grape::ipc::Session::Config{};
    config.mode = grape::ipc::Session::Mode::Client;
    const auto router_str = args.getOptionOrThrow<std::string>("router");
    if (router_str != "none") {
      config.router = grape::ipc::Locator::fromString(router_str);
      if (not config.router.has_value()) {
        grape::panic<grape::Exception>(std::format("Failed to parse router '{}' ", router_str));
      }
      std::println("Router: '{}'", router_str);
    }
    auto session = grape::ipc::Session(config);

    const auto message_callback = [](std::span<const std::byte> bytes) {
      // Reinterpret message as string
      std::println("{}", asString(bytes));
    };

    auto subs = session.createSubscriber(grape::ipc::ex::client::TOPIC, message_callback);
    std::println("Subscriber started on '{}'", grape::ipc::ex::client::TOPIC);

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
