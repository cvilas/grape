//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <print>

#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// The process of discovering Zenoh applications is called scouting. For a discussion of how
// scouting works in a deployment consisting of peers, clients and routers, see
// https://zenoh.io/docs/getting-started/deployment/
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/universal/z_scout.cxx
//=================================================================================================

//=================================================================================================
auto main() -> int {
  try {
    std::atomic_flag done_flag = ATOMIC_FLAG_INIT;

    const auto on_hello = [](const zenoh::Hello& hello) {
      std::println("Hello from pid: {}, WhatAmI: {}, locators: {}",
                   grape::ipc::toString(hello.get_id()), grape::ipc::toString(hello.get_whatami()),
                   hello.get_locators());
    };

    const auto on_good_bye = [&done_flag]() {
      done_flag.test_and_set();
      done_flag.notify_one();
    };

    std::println("Scouting..");
    auto config = zenoh::Config::create_default();
    static constexpr auto SCOUT_DURATION = std::chrono::milliseconds(4000);
    zenoh::scout(std::move(config), on_hello, on_good_bye,
                 { .timeout_ms = SCOUT_DURATION.count() });
    done_flag.wait(false);
    std::println("done");

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
