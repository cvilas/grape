//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <benchmark/benchmark.h>

#include "grape/clock/clock_broadcaster.h"
#include "grape/clock/follower_clock.h"
#include "grape/wall_clock.h"

namespace {

//-------------------------------------------------------------------------------------------------
void bmClockBroadcasterPost(benchmark::State& state) {
  const auto* const clock_name = "bm_clock_broadcaster";

  auto broadcaster = grape::clock::ClockBroadcaster({ .name = clock_name });
  auto tp = grape::clock::FollowerClock::TimePoint{};

  for (auto unused : state) {
    (void)unused;
    broadcaster.post(tp);
    benchmark::DoNotOptimize(tp);
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bmClockBroadcasterPost)->Unit(benchmark::kNanosecond);

//-------------------------------------------------------------------------------------------------
void bmFollowerClockNow(benchmark::State& state) {
  const auto* const clock_name = "bm_clock";

  auto clock = grape::clock::FollowerClock(clock_name);

  for (auto unused : state) {
    (void)unused;
    auto time_point = clock.now();
    benchmark::DoNotOptimize(time_point);
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bmFollowerClockNow)->Unit(benchmark::kNanosecond);

//-------------------------------------------------------------------------------------------------
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
