//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <semaphore>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/thread.h"

namespace {

TEST_CASE("Stopping a thread interrupts its periodic wait", "[Thread]") {
  using namespace std::chrono_literals;
  static constexpr auto PROCESS_INTERVAL = 5s;
  static constexpr auto MAX_STOP_DURATION = 2s;
  auto process_started = std::binary_semaphore{ 0 };
  auto config = grape::realtime::Thread::Config{};
  config.setup = [] { return true; };
  config.process = [&process_started] {
    process_started.release();
    return true;
  };
  config.interval = PROCESS_INTERVAL;
  auto thread = grape::realtime::Thread{ std::move(config) };
  thread.start();
  process_started.acquire();

  const auto start = std::chrono::steady_clock::now();
  thread.stop();
  const auto stop_duration = std::chrono::steady_clock::now() - start;

  REQUIRE(stop_duration < MAX_STOP_DURATION);
}

}  // namespace
