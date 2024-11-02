//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "examples_utils.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program creates a Zenoh router. Routers route data between 'clients' and local
// subnetworks of 'peers'. For a description of routers, see
// https://zenoh.io/docs/getting-started/deployment/
//
// Typical usage:
// router [--port=1234]
//
// Paired with: sub_client, pub_client
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

    const auto args_opt =
        grape::conio::ProgramDescription("Zenoh router")
            .declareOption<int>("port", "Port on which the service is available", DEFAULT_PORT)
            .declareOption<std::string>("address", "IP address of the service", DEFAULT_ADDRESS)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto port = grape::ipc::ex::getOptionOrThrow<int>(args, "port");
    const auto addr = grape::ipc::ex::getOptionOrThrow<std::string>(args, "address");

    // configure as router
    auto config = zenoh::Config::create_default();
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("router")");

    const auto listen_on = std::format(R"(["tcp/{}:{}"])", addr, port);
    config.insert_json5(Z_CONFIG_LISTEN_KEY, listen_on);

    // start session
    auto session = zenoh::Session::open(std::move(config));
    std::println("Router listening on {}", listen_on);
    std::println("PID: {}", grape::ipc::toString(session.get_zid()));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
