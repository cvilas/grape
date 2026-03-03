//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <csignal>
#include <print>
#include <thread>

#include "grape/ego_clock.h"
#include "grape/ego_clock_driver.h"
#include "grape/exception.h"
#include "grape/ipc/session.h"

namespace {
//-------------------------------------------------------------------------------------------------
void masterClock(const std::stop_token& st, const std::string& clock_name) {
  try {
    std::println("\nMaster clock start");
    static constexpr auto EGO_TICK_PERIOD = std::chrono::milliseconds(10);
    static constexpr auto WALL_TICK_PERIOD = std::chrono::milliseconds(100);
    const auto config =
        grape::EgoClockDriver::Config{ .clock_name = clock_name,
                                       .broadcast_interval = std::chrono::seconds(1),
                                       .calibration_window = 40U };
    auto driver = grape::EgoClockDriver(config);
    auto ego_time = grape::EgoClock::TimePoint{};
    while (not st.stop_requested()) {
      const auto wall_time = grape::WallClock::now();
      driver.tick(ego_time, wall_time);
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

    // initialise IPC, because clocks communicate over it
    grape::ipc::init({});

    static constexpr auto CLOCK_NAME = "example_clock";

    // create and run master clock
    const auto master = std::jthread(masterClock, CLOCK_NAME);

    // wait for master clock to initialise ego clock
    std::println("\nWaiting for clock to initialise");
    static constexpr auto MASTER_WAIT_TIME = std::chrono::seconds(10);
    auto ego_clock = grape::EgoClock::create(CLOCK_NAME, MASTER_WAIT_TIME);
    if (not ego_clock) {
      std::println("No master clock");
      return EXIT_FAILURE;
    }

    // run process loop using ego clock
    static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(100);
    while (!s_exit) {
      const auto ego_now = ego_clock->now();
      const auto wall_now = grape::WallClock::now();
      std::println("Time now: Ego: {}, Wall: {}", ego_now, wall_now);
      ego_clock->sleepUntil(ego_now + LOOP_PERIOD);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
