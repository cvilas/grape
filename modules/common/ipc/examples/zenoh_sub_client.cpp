//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that creates a 'client' subscriber. Clients connect to other peers and clients
// via routers. For a description of clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/.
//
// Typical usage:
// ```bash
// zenoh_sub_client [--key="topic_name" --router="ip:port"]
// ```
//
// Paired with: zenoh_pub_client.cpp, zenoh_router.cpp
//
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";
    static constexpr auto DEFAULT_ROUTER = "localhost:7447";

    auto desc = grape::conio::ProgramDescription("Example subscriber operating in 'client' mode");
    desc.defineOption<std::string>("key", "Key expression", DEFAULT_KEY)
        .defineOption<std::string>("router", "Router adress and port", DEFAULT_ROUTER);

    const auto args = std::move(desc).parse(argc, argv);
    const auto key = args.getOption<std::string>("key");
    const auto router = args.getOption<std::string>("router");

    zenohc::Config config;

    // configure as client
    if (not config.insert_json(Z_CONFIG_MODE_KEY, R"("client")")) {
      std::println("Setting mode failed");
      return EXIT_FAILURE;
    }

    // configure router to connect to
    const auto router_endpoint = std::format(R"(["tcp/{}"])", router);
    if (not config.insert_json(Z_CONFIG_CONNECT_KEY, router_endpoint.c_str())) {
      std::println("Error setting endpoint {}", router_endpoint);
      return EXIT_FAILURE;
    }

    // open session and print some info
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));
    std::println("Configured as client for router at {}", router_endpoint);

    const auto cb = [](const zenohc::Sample& sample) {
      std::println(">> Received {} ([{}] '{}' : '{}')", grape::ipc::toString(sample.get_kind()),
                   grape::ipc::toString(sample.get_timestamp()),
                   sample.get_keyexpr().as_string_view(), sample.get_payload().as_string_view());
    };

    auto subs = grape::ipc::expect<zenohc::Subscriber>(session.declare_subscriber(key, cb));
    std::println("Subscriber started on '{}'", subs.get_keyexpr().as_string_view());

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
