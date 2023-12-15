//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that puts a path/value into Zenoh. The path/value will be received by all
// matching subscribers.
//
// Typical usage:
// ```bash
// zenoh_put [--key="demo/example/test" --value="Hello World"]
// ```
//
// Paired with example: zenoh_sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_put.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";

    auto desc = grape::conio::ProgramDescription("Puts a specified value on specified key");
    desc.defineOption<std::string>("key", "Key expression", DEFAULT_KEY)
        .defineOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE);

    const auto args = std::move(desc).parse(argc, argv);
    const auto key = args.getOption<std::string>("key");
    const auto value = args.getOption<std::string>("value");

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Putting Data ('{}': '{}')...", key, value);
    zenohc::PutOptions options;
    options.set_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN);

    if (not session.put(key, value, options)) {
      std::println("Put failed");
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}
