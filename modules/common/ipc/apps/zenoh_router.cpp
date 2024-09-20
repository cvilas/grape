//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
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

    const auto args_opt =
        grape::conio::ProgramDescription("Zenoh router")
            .declareOption<int>("port", "Port on which the service is available", DEFAULT_PORT)
            .declareOption<std::string>("address", "IP address of the service", DEFAULT_ADDRESS)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();

    const auto port_opt = args.getOption<int>("port");
    if (not port_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ port_opt.error() };
    }
    const auto addr_opt = args.getOption<std::string>("address");
    if (not addr_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ addr_opt.error() };
    }

    // configure as router
    auto config = zenoh::Config::create_default();
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("router")");

    const auto listen_on = std::format(R"(["tcp/{}:{}"])", addr_opt.value(), port_opt.value());
    config.insert_json5(Z_CONFIG_LISTEN_KEY, listen_on);

    // start session
    auto session = zenoh::Session::open(std::move(config));
    std::println("Router listening on {}", listen_on);
    std::println("PID: {}", grape::ipc::toString(session.get_zid()));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    const auto code_str = toString(ex.code);
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "Option '%s' %.*s", ex.key.c_str(),
                          static_cast<int>(code_str.length()), code_str.data());
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
