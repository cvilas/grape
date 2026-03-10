//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <csignal>
#include <print>
#include <thread>

#include "grape/clock/clock_broadcaster.h"
#include "grape/clock/follower_clock.h"
#include "grape/exception.h"
#include "grape/wall_clock.h"

namespace {
//-------------------------------------------------------------------------------------------------
void masterClock(const std::stop_token& st, const std::string& clock_name) {
  try {
    std::println("\nMaster clock start");
    static constexpr auto EGO_TICK_PERIOD = std::chrono::milliseconds(10);
    static constexpr auto WALL_TICK_PERIOD = std::chrono::milliseconds(100);
    const auto config = grape::clock::ClockBroadcaster::Config{ .name = clock_name };
    auto driver = grape::clock::ClockBroadcaster(config);
    auto ego_time = grape::clock::FollowerClock::TimePoint{};
    while (not st.stop_requested()) {
      const auto wall_time = grape::WallClock::now();
      driver.post(ego_time);
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
    auto master = std::jthread(masterClock, CLOCK_NAME);

    // create and run follower clock
    auto clk = grape::clock::FollowerClock(CLOCK_NAME);

    // wait for signs of life from broadcaster
    static constexpr auto STARTUP_TIMEOUT = std::chrono::milliseconds(5000);
    if (not clk.waitForNextTick(STARTUP_TIMEOUT)) {
      std::println("Interrupted or timed out waiting for broadcaster");
      return EXIT_FAILURE;
    }

    static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(100);
    while (not s_exit) {
      const auto now = clk.now();
      const auto wall_now = grape::WallClock::now();
      std::println("Time now: {} (Wall: {})", now, wall_now);
      clk.sleepUntil(now + LOOP_PERIOD);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
