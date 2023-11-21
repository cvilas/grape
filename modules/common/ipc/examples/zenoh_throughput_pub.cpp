//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/ipc/ipc.h"
#include "grape/utils/command_line_args.h"

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
    const auto args = grape::utils::CommandLineArgs(argc, argv);

    static constexpr auto DEFAULT_PAYLOAD_SIZE = 8;
    static constexpr uint8_t DEFAULT_PAYLOAD_FILL = 1;
    const auto size_opt = args.getOption<size_t>("size");
    const auto payload_size = (size_opt.has_value() ? size_opt.value() : DEFAULT_PAYLOAD_SIZE);
    const auto value = std::vector<uint8_t>(payload_size, DEFAULT_PAYLOAD_FILL);
    std::println("Payload size [--size]: {} bytes", payload_size);

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

  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
