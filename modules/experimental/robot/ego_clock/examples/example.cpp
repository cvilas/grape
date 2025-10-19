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
void masterClock(const std::stop_token& st) {
  try {
    std::println("\nMaster clock start");
    static constexpr auto EGO_TICK_PERIOD = std::chrono::milliseconds(10);
    static constexpr auto WALL_TICK_PERIOD = std::chrono::milliseconds(100);
    static constexpr auto CONFIG = grape::EgoClockDriver::Config{
      .broadcast_interval = 20U,  // Broadcast clock sync every 20 ticks (1 second)
      .calibration_window = 40U   // Use 40 samples for clock fit
    };
    auto driver = grape::EgoClockDriver(CONFIG);
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

    // create and run master clock
    const auto master = std::jthread(masterClock);

    // wait for master clock to initialise ego clock
    std::println("\nWaiting for clock to initialise");
    static constexpr auto MASTER_WAIT_TIME = std::chrono::seconds(10);
    if (not grape::EgoClock::waitForMaster(MASTER_WAIT_TIME)) {
      std::println("No master clock");
      return EXIT_FAILURE;
    }

    // run process loop using ego clock
    static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(100);
    while (!s_exit) {
      const auto ego_now = grape::EgoClock::now();
      const auto wall_now = grape::WallClock::now();
      std::println("Time now: Ego: {}, Wall: {}", ego_now, wall_now);
      grape::sleepUntil(ego_now + LOOP_PERIOD);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
