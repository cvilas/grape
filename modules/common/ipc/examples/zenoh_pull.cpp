//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/conio/conio.h"
#include "grape/exception.h"
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

    const auto args_opt = grape::conio::ProgramDescription("Pulls data on specified key on-demand")
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
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    std::ignore = std::fputs(toString(ex).c_str(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}