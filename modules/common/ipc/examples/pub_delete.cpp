//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Example program that publishes a sample with Samplekind::Delete. Such messages can be
// interpreted by subscribers and storages however they want, such as "delete all data for keys
// included by this sample's key expression".
//
// Typical usage:
// ```code
// pub_delete [--key="demo/**"]
// ```
//
// Paired with example: sub.cpp
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
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = args.getOptionOrThrow<std::string>("key");

    std::println("Opening session...");
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    std::println("Deleting resources matching '{}'...", key);
    session.delete_resource(key);

    // Note: The same result can be accomplished by calling delete_resource() on a publisher.

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
