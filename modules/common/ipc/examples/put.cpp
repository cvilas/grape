//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that puts a path/value into Zenoh. The path/value will be received by all
// matching subscribers.
//
// Typical usage:
// ```bash
// put [--key="demo/example/test" --value="Hello World"]
// ```
//
// Paired with example: sub.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_put.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";
    static constexpr auto DEFAULT_VALUE = "Put from Zenoh C++!";

    const auto args_opt =
        grape::conio::ProgramDescription("Puts a specified value on specified key")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto value = grape::ipc::ex::getOptionOrThrow<std::string>(args, "value");

    std::println("Opening session...");
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    std::println("Putting Data ('{}': '{}')...", key, value);
    session.put(key, zenoh::ext::serialize(value), { .encoding = zenoh::Encoding("text/plain") });

    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    std::ignore = std::fputs(toString(ex).c_str(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
