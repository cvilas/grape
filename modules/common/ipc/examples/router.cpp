//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <string>

#include "grape/conio/program_options.h"
#include "grape/ipc/common.h"
#include "zenoh_utils.h"

//=================================================================================================
// Creates a Zenoh router. Routers route data between 'clients' and local subnetworks of 'peers'.
// For a description of routers, see https://zenoh.io/docs/getting-started/deployment/
//
// Typical usage:
// router [--locator=proto/address:port]
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

    static constexpr auto DEFAULT_LOCATOR = "tcp/[::]:7447";

    const auto maybe_args =
        grape::conio::ProgramDescription("Zenoh router")
            .declareOption<std::string>("locator", "Server endpoint", DEFAULT_LOCATOR)
            .parse(argc, argv);

    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();

    const auto locator_arg = args.getOptionOrThrow<std::string>("locator");
    const auto maybe_locator = grape::ipc::Locator::fromString(locator_arg);
    if (not maybe_locator.has_value()) {
      grape::panic<grape::Exception>(std::format("Failed to parse locator '{}'", locator_arg));
    }
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto locator_str = std::format(R"(["{}"])", toString(maybe_locator.value()));

    // configure as router
    auto config = zenoh::Config::create_default();
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("router")");
    config.insert_json5(Z_CONFIG_LISTEN_KEY, locator_str);

    // start session
    auto session = zenoh::Session::open(std::move(config));
    std::println("Router listening on {}", locator_str);
    std::println("PID: {}", grape::ipc::ex::toString(session.get_zid()));

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
