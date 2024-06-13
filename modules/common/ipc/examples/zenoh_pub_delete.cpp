//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "examples_utils.h"
#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program that publishes a sample with Samplekind::Delete. Such messages can be
// interpreted by subscribers and storages however they want, such as "delete all data for keys
// included by this sample's key expression". Run this program with zenoh_sub in another terminal
// to witness the interaction.
//
// Typical usage:
// ```code
// zenoh_pub_delete [--key="demo/**"]
// ```
//
// Paired with example: zenoh_sub.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_delete.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Notifies deletion of data on specified key")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Deleting resources matching '{}'...", key);
    zenohc::ErrNo error{};
    if (not session.delete_resource(key, error)) {
      std::println("Delete failed with error {}", error);
      return EXIT_FAILURE;
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
