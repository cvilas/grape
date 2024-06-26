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
// zenoh_pub_client [--key="topic_name" --value="string text" --router="ip:port"]
// ```
//
// Paired with: zenoh_sub_client.cpp, zenoh_router.cpp
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
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();

    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto value = grape::ipc::ex::getOptionOrThrow<std::string>(args, "value");
    const auto router = grape::ipc::ex::getOptionOrThrow<std::string>(args, "router");

    zenohc::Config config;

    // configure as client
    if (not config.insert_json(Z_CONFIG_MODE_KEY, R"("client")")) {
      std::println("Setting client mode failed");
      return EXIT_FAILURE;
    }

    // configure router to connect to.
    const auto router_endpoint = std::format(R"(["tcp/{}"])", router);
    if (not config.insert_json(Z_CONFIG_CONNECT_KEY, router_endpoint.c_str())) {
      std::println("Error setting endpoint {}", router_endpoint);
      return EXIT_FAILURE;
    }

    // open session and print some info
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));
    std::println("Configured as client for router at {}", router_endpoint);

    auto pub = grape::ipc::expect<zenohc::Publisher>(session.declare_publisher(key));
    std::println("Publisher created on '{}'", key);

    zenohc::PublisherPutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    std::println("Press ctrl-c to exit");
    while (not s_exit) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      pub.put(msg);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    std::ignore = std::fputs(toString(ex).c_str(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}