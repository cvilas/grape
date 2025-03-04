//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Demonstrative example that subscribes to liveliness changes on tokens that match a given key
// expression (grape/ipc2/example/zenoh/** by default). To learn how tokens are declared, see
// liveliness_declare.
//
// Note: This example will not receive information about matching liveliness tokens that were alive
// before it's start.
//
// Typical usage:
// ```bash
// liveliness_sub [--key=key/expression/**]
// ```
//
// Paired with example: liveliness_declare
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_sub_liveliness.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc2/example/zenoh/**";

    const auto args_opt =
        grape::conio::ProgramDescription("Subscribes to liveliness state changes")
            .declareOption<std::string>("key", "key expression to track liveliness of", DEFAULT_KEY)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();

    std::println("Opening session...");
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    const auto key = args.getOptionOrThrow<std::string>("key");

    const auto cb = [](const zenoh::Sample& sample) {
      switch (sample.get_kind()) {
        case Z_SAMPLE_KIND_PUT:
          std::println(">> New alive token ('{}')", sample.get_keyexpr().as_string_view());
          break;
        case Z_SAMPLE_KIND_DELETE:
          std::println(">> Dropped token ('{}')", sample.get_keyexpr().as_string_view());
          break;
      }
    };

    std::println("Declaring liveliness subscriber on '{}'...", key);
    [[maybe_unused]] auto sub =
        session.liveliness_declare_subscriber(key, cb, zenoh::closures::none);

    std::println("Press CTRL-C to quit...");
    while (true) {
      static constexpr auto LOOP_WAIT = std::chrono::milliseconds(1000);
      std::this_thread::sleep_for(LOOP_WAIT);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
