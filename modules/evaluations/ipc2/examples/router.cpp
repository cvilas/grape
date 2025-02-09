//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <string>

#include "grape/conio/program_options.h"
#include "grape/ipc2/common.h"
#include "zenoh_utils.h"

//=================================================================================================
// Creates a Zenoh router. Routers route data between 'clients' and local subnetworks of 'peers'.
// Routers can connect to other routers to pass data across isolated networks
// For a description of routers, see https://zenoh.io/docs/getting-started/deployment/
//
// Typical usage:
// router [--listen_on=proto/address:port --connect_to=proto/address:port]
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

    static constexpr auto LISTEN_ON = "tcp/[::]:7447";
    static constexpr auto CONNECT_TO = "";

    const auto maybe_args =
        grape::conio::ProgramDescription("Zenoh router")
            .declareOption<std::string>(
                "listen_on", "Locator to listen for connections from Zenoh sessions and routers",
                LISTEN_ON)
            .declareOption<std::string>(
                "connect_to", "Locator of another Zenoh session or router to connect to directly",
                CONNECT_TO)
            .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    // Set locator to listen for connections from Zenoh sessions
    const auto listen_on_arg = args.getOptionOrThrow<std::string>("listen_on");
    const auto maybe_listen_on = grape::ipc2::Locator::fromString(listen_on_arg);
    if (not maybe_listen_on.has_value()) {
      grape::panic<grape::Exception>(
          std::format("Failed to parse 'listen_on' locator '{}'", listen_on_arg));
    }
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto listen_on_str = std::format(R"(["{}"])", toString(maybe_listen_on.value()));

    // build configuration for router
    auto config = zenoh::Config::create_default();
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("router")");
    config.insert_json5(Z_CONFIG_LISTEN_KEY, listen_on_str);

    // Set locator of another Zenoh session or router to connect to
    const auto connect_to_arg = args.getOptionOrThrow<std::string>("connect_to");
    if (not connect_to_arg.empty()) {
      const auto maybe_connect_to = grape::ipc2::Locator::fromString(connect_to_arg);
      if (not maybe_connect_to.has_value()) {
        grape::panic<grape::Exception>(
            std::format("Failed to parse 'connect_to' locator '{}'", connect_to_arg));
      }
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
      const auto connect_to_str = std::format(R"(["{}"])", toString(maybe_connect_to.value()));
      config.insert_json5(Z_CONFIG_CONNECT_KEY, connect_to_str);
    }

    // start session
    auto session = zenoh::Session::open(std::move(config));
    std::println("Router");
    std::println("\tListens on {}", listen_on_arg);
    std::println("\tConnects to {}", connect_to_arg.empty() ? "(unspecified)" : connect_to_arg);
    std::println("\tPID: {}", grape::ipc2::ex::toString(session.get_zid()));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
