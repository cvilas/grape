//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <csignal>
#include <print>
#include <thread>

#include "grape/clock_driver.h"
#include "grape/exception.h"
#include "grape/follower_clock.h"
#include "grape/wall_clock.h"

namespace {
//-------------------------------------------------------------------------------------------------
void masterClock(const std::stop_token& st, const std::string& clock_name) {
  try {
    std::println("\nMaster clock start");
    static constexpr auto EGO_TICK_PERIOD = std::chrono::milliseconds(10);
    static constexpr auto WALL_TICK_PERIOD = std::chrono::milliseconds(100);
    const auto config = grape::ClockDriver::Config{ .clock_name = clock_name };
    auto driver = grape::ClockDriver(config);
    auto ego_time = grape::FollowerClock::TimePoint{};
    while (not st.stop_requested()) {
      const auto wall_time = grape::WallClock::now();
      driver.tick(ego_time);
      std::this_thread::sleep_until(wall_time + WALL_TICK_PERIOD);
      ego_time += EGO_TICK_PERIOD;
    }

    std::println("\nMaster clock exit");
  } catch (...) {
    grape::Exception::print();
  }
}

//-------------------------------------------------------------------------------------------------
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit = true;
}
}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    (void)signal(SIGINT, onSignal);
    (void)signal(SIGTERM, onSignal);

    static constexpr auto CLOCK_NAME = "example_clock";

    // create and run master clock
    const auto master = std::jthread(masterClock, CLOCK_NAME);

    // wait for master clock to initialise ego clock
    std::println("\nWaiting for clock to initialise");
    static constexpr auto MASTER_WAIT_TIME = std::chrono::seconds(10);
    auto clk = grape::FollowerClock::create(CLOCK_NAME, MASTER_WAIT_TIME);
    if (not clk) {
      std::println("No master clock");
      return EXIT_FAILURE;
    }

    // run process loop using ego clock
    static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(100);
    while (!s_exit) {
      const auto ego_now = clk->now();
      const auto wall_now = grape::WallClock::now();
      std::println("Time now: Ego: {}, Wall: {}", ego_now, wall_now);
      clk->sleepUntil(ego_now + LOOP_PERIOD);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
