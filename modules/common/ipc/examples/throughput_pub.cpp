//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"
#include "throughput_constants.h"

//=================================================================================================
// Publishing end of the pair of example programs to measure throughput between a publisher and a
// subscriber.
//
// Typical usage:
// ```code
// throughput_pub [--size=8 --router="proto/address:port"]
// ```
//
// Paired with example: throughput_sub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_pub_thr.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_PAYLOAD_SIZE = 8;
    static constexpr char DEFAULT_PAYLOAD_FILL{ 0x01 };

    const auto maybe_args =
        grape::conio::ProgramDescription("Publisher end of throughput measurement example")
            .declareOption<std::size_t>("size", "payload size in bytes", DEFAULT_PAYLOAD_SIZE)
            .declareOption<std::string>("router", "Router locator", "none")
            .parse(argc, argv);
    if (not maybe_args.has_value()) {
      grape::panic<grape::Exception>(toString(maybe_args.error()));
    }
    const auto& args = maybe_args.value();
    const auto payload_size = args.getOptionOrThrow<size_t>("size");
    const auto value = std::vector<char>(payload_size, DEFAULT_PAYLOAD_FILL);
    std::println("Payload size: {} bytes", payload_size);

    // prepare session
    auto config = grape::ipc::Session::Config{};
    const auto router_str = args.getOptionOrThrow<std::string>("router");
    if (router_str != "none") {
      // If a router is specified, turn this into a client and connect to the router
      config.mode = grape::ipc::Session::Mode::Client;
      config.router = grape::ipc::Locator::fromString(router_str);
      if (not config.router.has_value()) {
        grape::panic<grape::Exception>(std::format("Failed to parse router '{}' ", router_str));
      }
      std::println("Router: '{}'", router_str);
    }
    auto session = grape::ipc::Session(config);

    auto pub = session.createPublisher({ .key = grape::ipc::ex::throughput::TOPIC });

    std::println("Press CTRL-C to quit");
    while (true) {
      pub.publish(value);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
