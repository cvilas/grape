//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Declares a liveliness token on a given key expression. This token will be seen alive by the
// subscribers until the token is undeclared.
//
// Typical usage:
// ```bash
// liveliness_declare [--key=group1/member1]
// ```
//
// Paired with example: liveliness_get, liveliness_sub
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_liveliness.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc2/example/zenoh/liveliness";

    const auto args_opt =
        grape::conio::ProgramDescription("Declares/undeclares liveliness token")
            .declareOption<std::string>("key", "key expression to declare liveliness token on",
                                        DEFAULT_KEY)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();

    std::println("Opening session...");
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));
    const auto key = args.getOptionOrThrow<std::string>("key");

    std::println("Declaring liveliness token for '{}'", key);
    [[maybe_unused]] const auto token = session.liveliness_declare_token(key);

    std::println("Press CTRL-C to undeclare token and quit");
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
