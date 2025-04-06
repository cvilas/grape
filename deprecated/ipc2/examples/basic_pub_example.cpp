//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "basic_example_constants.h"
#include "grape/conio/program_options.h"
#include "grape/ipc2/session.h"

//=================================================================================================
// Example program creates a publisher and periodically writes a value on the specified key. The
// published value will be received by all matching subscribers.
//
// Typical usage:
// ```bash
// basic_pub_example [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: basic_sub_example.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_pub.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";

    const auto args_opt =
        grape::conio::ProgramDescription("Periodic publisher example")
            .declareOption<std::string>("key", "Key expression", grape::ipc2::ex::DEFAULT_TOPIC)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = args.getOptionOrThrow<std::string>("key");
    const auto value = args.getOptionOrThrow<std::string>("value");

    std::println("Opening session...");
    auto session = grape::ipc2::Session({});

    std::println("Declaring Publisher on '{}'", key);
    auto pub = session.createPublisher({ .key = key });

    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    while (true) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      pub.publish({ msg.data(), msg.size() });
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
