//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that creates a 'client' subscriber. Clients connect to other peers and clients
// via routers. For a description of clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/.
//
// Typical usage:
// ```bash
// sub_client [--key="topic_name" --router="ip:port"]
// ```
//
// Paired with: pub_client.cpp, router.cpp
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

    const auto args_opt =
        grape::conio::ProgramDescription("Example subscriber operating in 'client' mode")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("router", "Router adress and port", DEFAULT_ROUTER)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto router = grape::ipc::ex::getOptionOrThrow<std::string>(args, "router");

    auto config = zenoh::Config::create_default();

    // configure as client
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("client")");

    // configure router to connect to
    const auto router_endpoint = std::format(R"(["tcp/{}"])", router);
    config.insert_json5(Z_CONFIG_CONNECT_KEY, router_endpoint);

    // open session and print some info
    auto session = zenoh::Session::open(std::move(config));
    std::println("Configured as client for router at {}", router_endpoint);

    const auto cb = [](const zenoh::Sample& sample) {
      const auto ts = sample.get_timestamp();
      std::println(">> Received {} ([{}] '{}' : '{}')", grape::ipc::toString(sample.get_kind()),
                   (ts ? grape::ipc::toString(ts.value()) : "--no timestamp--"),
                   sample.get_keyexpr().as_string_view(), sample.get_payload().as_string());
    };

    auto subs = session.declare_subscriber(key, cb, zenoh::closures::none);
    std::println("Subscriber started on '{}'", subs.get_keyexpr().as_string_view());

    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
