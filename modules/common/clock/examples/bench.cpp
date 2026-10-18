//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <thread>

#include <benchmark/benchmark.h>

#include "grape/clock/clock_broadcaster.h"
#include "grape/clock/follower_clock.h"
#include "grape/wall_clock.h"

namespace {

constexpr auto TICK_PERIOD =
    std::chrono::milliseconds(10);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
void bmClockBroadcasterPost(benchmark::State& state) {
  const auto* const clock_name = "bm_clock_broadcaster";

  auto broadcaster = grape::clock::ClockBroadcaster({ .name = clock_name });
  auto tp = grape::clock::FollowerClock::TimePoint{};

  for (const auto unused : state) {
    (void)unused;
    tp += TICK_PERIOD;
    broadcaster.post(tp);
    benchmark::DoNotOptimize(tp);
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bmClockBroadcasterPost)->Unit(benchmark::kNanosecond);

//-------------------------------------------------------------------------------------------------
void bmFollowerClockNow(benchmark::State& state) {
  static constexpr auto CLOCK_NAME = "bm_clock_now";

  auto broadcaster = grape::clock::ClockBroadcaster({ .name = CLOCK_NAME });
  const auto clock = grape::clock::FollowerClock(CLOCK_NAME);
  auto tp = grape::clock::FollowerClock::TimePoint{};
  broadcaster.post(tp);
  const auto tick_thread =
      std::jthread([&broadcaster, tp](const std::stop_token& stop_token) mutable {
        while (not stop_token.stop_requested()) {
          tp += TICK_PERIOD;
          broadcaster.post(tp);
          std::this_thread::sleep_for(TICK_PERIOD);
        }
      });

  for (const auto unused : state) {
    (void)unused;
    auto time_point = clock.now();
    benchmark::DoNotOptimize(time_point);
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bmFollowerClockNow)->Unit(benchmark::kNanosecond);

//-------------------------------------------------------------------------------------------------
void bmWallClockNow(benchmark::State& state) {
  for (const auto unused : state) {
    (void)unused;
    auto wall_time = grape::WallClock::now();
    benchmark::DoNotOptimize(wall_time);
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(bmWallClockNow)->Unit(benchmark::kNanosecond);

}  // namespace

BENCHMARK_MAIN();
