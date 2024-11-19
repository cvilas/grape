//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <string>

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Creates a Zenoh router. Routers route data between 'clients' and local subnetworks of 'peers'.
// For a description of routers, see https://zenoh.io/docs/getting-started/deployment/
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
    static constexpr auto DEFAULT_ADDRESS = "[::]";  //!< use all available interfaces

    const auto maybe_args =
        grape::conio::ProgramDescription("Zenoh router")
            .declareOption<std::uint16_t>("port", "IP port for the service", DEFAULT_PORT)
            .declareOption<std::string>("address", "IP address of the service", DEFAULT_ADDRESS)
            .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();
    const auto port = args.getOptionOrThrow<std::uint16_t>("port");
    const auto addr = args.getOptionOrThrow<std::string>("address");

    // configure as router
    auto config = zenoh::Config::create_default();
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("router")");

    const auto listen_on = std::format(R"(["tcp/{}:{}"])", addr, port);
    config.insert_json5(Z_CONFIG_LISTEN_KEY, listen_on);

    // start session
    auto session = zenoh::Session::open(std::move(config));
    std::println("Router listening on {}", listen_on);
    std::println("PID: {}", grape::ipc::ex::toString(session.get_zid()));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
