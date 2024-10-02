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
// throughput_pub [--size=8]
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

    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    static constexpr auto TOPIC = "grape/ipc/example/zenoh/throughput";
    auto pub =
        session.declare_publisher(TOPIC, { .congestion_control = Z_CONGESTION_CONTROL_BLOCK });

    std::println("Press CTRL-C to quit");
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
