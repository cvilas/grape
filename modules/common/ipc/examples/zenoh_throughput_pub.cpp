//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Publishing end of the pair of example programs to measure throughput between a publisher and a
// subscriber.
//
// Typical usage:
// ```code
// zenoh_throughput_pub [--size=8]
// ```
//
// Paired with example: zenoh_throughput_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_pub_thr.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_PAYLOAD_SIZE = 8;
    static constexpr uint8_t DEFAULT_PAYLOAD_FILL = 1;

    const auto args_opt =
        grape::conio::ProgramDescription("Publisher end of throughput measurement example")
            .declareOption<size_t>("size", "payload size in bytes", DEFAULT_PAYLOAD_SIZE)
            .parse(argc, argv);
    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto payload_size = grape::ipc::ex::getOptionOrThrow<size_t>(args, "size");
    const auto value = std::vector<uint8_t>(payload_size, DEFAULT_PAYLOAD_FILL);
    std::println("Payload size: {} bytes", payload_size);

    auto config = zenohc::Config();
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    static constexpr auto TOPIC = "grape/ipc/example/zenoh/throughput";
    zenohc::PublisherOptions popt;
    popt.set_congestion_control(zenohc::CongestionControl::Z_CONGESTION_CONTROL_BLOCK);
    auto pub = grape::ipc::expect<zenohc::Publisher>(session.declare_publisher(TOPIC, popt));

    while (true) {
      pub.put(value);
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
