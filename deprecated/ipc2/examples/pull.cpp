//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>


#include "zenoh_utils.h"
#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"

//=================================================================================================
// Example program demonstrates a subscriber that is notified of last put/delete by polling
// on-demand, rather than being event-triggered on message arrival.
//
// Typical usage:
// ```bash
// pull [--key="demo/**"]
// ```
//
// Paired with example: put.cpp, pub_delete.cpp
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_pull.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc2/example/zenoh/put";

    const auto args_opt = grape::conio::ProgramDescription("Pulls data on specified key on-demand")
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

    const auto cb = [](const zenoh::Sample& sample) {
      std::println(">> Received {} ('{}' : '{}')", grape::ipc2::ex::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(), sample.get_payload().as_string());
    };

    std::println("Declaring PullSubscriber on '{}'...", key);
    auto sub = session.declare_subscriber(key, cb, zenoh::closures::none);

    std::println("Press any key to pull data... and 'q' to quit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (true) {
      const auto c = grape::conio::kbhit() ? grape::conio::getch() : 0;
      if (c == 'q') {
        break;
      }
      if (c != 0) {
        sub.pull();
      }
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}