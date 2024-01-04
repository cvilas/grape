//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program creates a Zenoh router. Routers route data between 'clients' and local
// subnetworks of 'peers'. For a description of routers, see
// https://zenoh.io/docs/getting-started/deployment/
//
// Typical usage:
// zenoh_router [--port=1234]
//
// Paired with: zenoh_sub_client, zenoh_pub_client
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

    static constexpr auto DEFAULT_PORT = 7447;
    static constexpr auto DEFAULT_ADDRESS = "[::]";  //!< all available interfaces

    auto desc = grape::conio::ProgramDescription("Zenoh router");
    desc.defineOption<int>("port", "Port on which the service is available", DEFAULT_PORT)
        .defineOption<std::string>("address", "IP address of the service", DEFAULT_ADDRESS);

    const auto args = std::move(desc).parse(argc, argv);
    const auto port = args.getOption<int>("port");
    const auto addr = args.getOption<std::string>("address");

    zenohc::Config config;

    // configure as router
    if (not config.insert_json(Z_CONFIG_MODE_KEY, R"("router")")) {
      std::println("Setting mode failed");
      return EXIT_FAILURE;
    }

    const auto listener_endpoint = std::format(R"(["tcp/{}:{}"])", addr, port);
    if (not config.insert_json(Z_CONFIG_LISTEN_KEY, listener_endpoint.c_str())) {
      std::println("Setting listening to {} failed", listener_endpoint);
      return EXIT_FAILURE;
    }

    // start session
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));
    std::println("Router listening on {}", listener_endpoint);
    std::println("PID: {}", grape::ipc::toString(session.info_zid()));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
