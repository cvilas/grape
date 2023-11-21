//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/ipc/ipc.h"
#include "grape/utils/command_line_args.h"

//=================================================================================================
// Example program creates a publisher and periodically writes a value on the specified key. The
// published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// zenoh_pub [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: zenoh_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_pub.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args = grape::utils::CommandLineArgs(argc, argv);
    const auto key_opt = args.getOption<std::string>("key");
    const auto value_opt = args.getOption<std::string>("value");

    const auto& key = key_opt.has_value() ? key_opt.value() : DEFAULT_KEY;
    const auto& value = value_opt.has_value() ? value_opt.value() : DEFAULT_VALUE;

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Declaring Publisher on '{}'", key);
    auto pub = grape::ipc::expect<zenohc::Publisher>(session.declare_publisher(key));

    zenohc::PublisherPutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    while (true) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      pub.put(msg);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}