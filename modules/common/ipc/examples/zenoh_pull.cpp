//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Example program demonstrates a subscriber that is notified of last put/delete by polling
// on-demand, rather than being event-triggered on message arrival.
//
// Typical usage:
// ```bash
// zenoh_pull [--key="demo/**"]
// ```
//
// Paired with example: zenoh_put.cpp, zenoh_pub_delete.cpp
//
// Derived from
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_pull.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    auto desc = grape::conio::ProgramDescription("Pulls data on specified key on-demand");
    desc.defineOption<std::string>("key", "Key expression", DEFAULT_KEY);

    const auto args = std::move(desc).parse(argc, argv);
    const auto key = args.getOption<std::string>("key");

    zenohc::Config config;
    std::println("Opening session...");
    auto session = grape::ipc::expect<zenohc::Session>(open(std::move(config)));

    std::println("Declaring PullSubscriber on '{}'...", key);
    const auto cb = [](const zenohc::Sample& sample) {
      std::println(">> Received {} ('{}' : '{}')", grape::ipc::toString(sample.get_kind()),
                   sample.get_keyexpr().as_string_view(), sample.get_payload().as_string_view());
    };

    auto sub = grape::ipc::expect<zenohc::PullSubscriber>(session.declare_pull_subscriber(key, cb));

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
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}