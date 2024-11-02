//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that creates a 'client' publisher. Clients connect to other peers and clients
// via routers. For a description of clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/. This publisher periodically writes a value on
// the specified key. The published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// pub_client [--key="topic_name" --value="string text" --router="ip:port"]
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

    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";
    static constexpr auto DEFAULT_ROUTER = "localhost:7447";

    const auto args_opt =
        grape::conio::ProgramDescription("Example publisher operating in 'client' mode")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .declareOption<std::string>("router", "Router adress and port", DEFAULT_ROUTER)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();

    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto value = grape::ipc::ex::getOptionOrThrow<std::string>(args, "value");
    const auto router = grape::ipc::ex::getOptionOrThrow<std::string>(args, "router");

    auto config = zenoh::Config::create_default();

    // configure as client
    config.insert_json5(Z_CONFIG_MODE_KEY, R"("client")");

    // configure router to connect to.
    const auto router_endpoint = std::format(R"(["tcp/{}"])", router);
    config.insert_json5(Z_CONFIG_CONNECT_KEY, router_endpoint);

    // open session and print some info
    auto session = zenoh::Session::open(std::move(config));
    std::println("Configured as client for router at {}", router_endpoint);

    auto pub = session.declare_publisher(key);
    std::println("Publisher created on '{}'", key);

    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    std::println("Press ctrl-c to exit");
    while (not s_exit) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      pub.put(zenoh::ext::serialize(msg), { .encoding = zenoh::Encoding("text/plain") });
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
