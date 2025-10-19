//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

//=================================================================================================
// Example program to demonstrate querying subscriber.
//
// A querying subscriber queries, then subscribes. When matched with a caching publisher, the
// subscriber first performs a get to obtain the "existing" state, and thereafter continues
// behaving like a regular subscriber. Caching publisher plus querying subscribers are useful where
// a subscriber might join late (after publisher has started), but must receive historical data
// from a publisher in order to initialise and function.
//
// Paired with example: pub_cache.cpp
//
// Derived from: https://github.com/eclipse-zenoh/zenoh-c/blob/master/examples/z_query_sub.c
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc2/example/zenoh/put";

    const auto args_opt = grape::conio::ProgramDescription("Querying subscriber example")
                              .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
                              .parse(argc, argv);

    if (not args_opt.has_value()) {
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = args.getOptionOrThrow<std::string>("key");
    std::println("Declaring querying subscriber on '{}'...", key);

    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    const auto cb = [](const zenoh::Sample& sample) {
      const auto ts = sample.get_timestamp();
      std::println(">> Received {} ('{}' : [{}] '{}')",
                   grape::ipc2::ex::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(),
                   (ts ? grape::ipc2::ex::toString(ts.value()) : "--no timestamp--"),
                   sample.get_payload().as_string());
    };

    auto sub = session.declare_querying_subscriber(key, cb, zenoh::closures::none);

    std::println("Press any key to exit");
    static constexpr auto LOOP_WAIT = std::chrono::milliseconds(100);
    while (not grape::conio::kbhit()) {
      std::this_thread::sleep_for(LOOP_WAIT);
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
