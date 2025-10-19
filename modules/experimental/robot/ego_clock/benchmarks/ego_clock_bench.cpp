//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <memory>
#include <print>
#include <thread>

#include <benchmark/benchmark.h>

#include "grape/ego_clock.h"
#include "grape/ego_clock_driver.h"
#include "grape/exception.h"
#include "grape/ipc/session.h"

namespace {

//-------------------------------------------------------------------------------------------------
void masterClock(const std::stop_token& st, const std::string& clock_name) {
  try {
    std::println("Master clock start");
    static constexpr auto EGO_TICK_PERIOD = std::chrono::milliseconds(10);
    static constexpr auto WALL_TICK_PERIOD = std::chrono::milliseconds(100);
    const auto config = grape::EgoClockDriver::Config{
      .clock_name = clock_name,
      .broadcast_interval = 10U,  // Broadcast clock sync every 20 ticks (1 second)
      .calibration_window = 40U   // Use 40 samples for clock fit
    };
    auto driver = grape::EgoClockDriver(config);
    auto ego_time = grape::EgoClock::TimePoint{};
    while (not st.stop_requested()) {
      const auto wall_time = grape::WallClock::now();
      driver.tick(ego_time, wall_time);
      std::this_thread::sleep_until(wall_time + WALL_TICK_PERIOD);
      ego_time += EGO_TICK_PERIOD;
    }
    std::println("Master clock exit");
  } catch (...) {
    grape::Exception::print();
  }
}

//-------------------------------------------------------------------------------------------------
// Benchmark EgoClock::now()
void bmEgoClockNow(benchmark::State& state) {
  static auto is_ipc_init = false;
  if (not is_ipc_init) {
    grape::ipc::init({});
    is_ipc_init = true;
  }

  static auto count = 0;
  const auto clock_name = "bm_clock_" + std::to_string(count++);

  auto master = std::jthread(masterClock, clock_name);
  static constexpr auto MASTER_WAIT_TIME = std::chrono::seconds(10);
  auto ego_clock = grape::EgoClock::create(clock_name, MASTER_WAIT_TIME);
  if (not ego_clock) {
    std::println("No master clock");
    return;
  }

  for (auto unused : state) {
    (void)unused;
    auto time_point = ego_clock->now();
    benchmark::DoNotOptimize(time_point);
  }
  master.request_stop();
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bmEgoClockNow)->Unit(benchmark::kNanosecond);

//-------------------------------------------------------------------------------------------------
// Benchmark WallClock::now() for comparison
void bmWallClockNow(benchmark::State& state) {
  for (auto unused : state) {
    (void)unused;
    auto wall_time = grape::WallClock::now();
    benchmark::DoNotOptimize(wall_time);
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bmWallClockNow)->Unit(benchmark::kNanosecond);

}  // namespace

BENCHMARK_MAIN();
