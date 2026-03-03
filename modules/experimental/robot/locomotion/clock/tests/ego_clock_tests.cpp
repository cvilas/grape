//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <thread>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/ego_clock.h"
#include "grape/ego_clock_driver.h"
#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/wall_clock.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock time conversion utilities", "[ego_clock]") {
  SECTION("toNanos and fromNanos are inverse operations") {
    const auto nanos = 123456789LL;
    const auto tp = grape::EgoClock::fromNanos(nanos);
    REQUIRE(grape::EgoClock::toNanos(tp) == nanos);
  }

  SECTION("Zero nanoseconds maps to epoch") {
    const auto tp = grape::EgoClock::fromNanos(0);
    REQUIRE(grape::EgoClock::toNanos(tp) == 0);
  }

  SECTION("Negative nanoseconds are handled correctly") {
    const auto nanos = -123456789LL;
    const auto tp = grape::EgoClock::fromNanos(nanos);
    REQUIRE(grape::EgoClock::toNanos(tp) == nanos);
  }

  SECTION("Large nanosecond values are handled correctly") {
    const auto nanos = 9223372036854775LL;
    const auto tp = grape::EgoClock::fromNanos(nanos);
    REQUIRE(grape::EgoClock::toNanos(tp) == nanos);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock time point arithmetic", "[ego_clock]") {
  SECTION("Time point addition with duration") {
    const auto tp1 = grape::EgoClock::fromNanos(1000000000LL);
    const auto duration = std::chrono::milliseconds(500);
    const auto tp2 = tp1 + duration;

    const auto expected_nanos = 1000000000LL + 500000000LL;
    REQUIRE(grape::EgoClock::toNanos(tp2) == expected_nanos);
  }

  SECTION("Duration between time points") {
    const auto tp1 = grape::EgoClock::fromNanos(1000000000LL);
    const auto tp2 = grape::EgoClock::fromNanos(2000000000LL);
    const auto duration = tp2 - tp1;

    const auto expected_nanos = 1000000000LL;
    REQUIRE(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() ==
            expected_nanos);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock operation with master clock", "[ego_clock]") {
  using namespace std::chrono_literals;

  grape::ipc::init({});  // Easy to forget. Clocks depend on IPC

  static constexpr auto TEST_CLOCK_NAME = "test_clock";

  // Without master clock won't initialise
  REQUIRE_FALSE(grape::EgoClock::create(TEST_CLOCK_NAME, 10ms));

  // Create a master clock driver
  const auto master_clock = [](const std::stop_token& st, const std::string& clock_name) {
    try {
      static constexpr auto EGO_TICK_PERIOD = 10ms;
      static constexpr auto WALL_TICK_PERIOD = 100ms;
      const auto config = grape::EgoClockDriver::Config{
        .clock_name = clock_name,
        .broadcast_interval = std::chrono::milliseconds(EGO_TICK_PERIOD * 2),
        .calibration_window = 2U  // Minimal window for testing
      };
      auto driver = grape::EgoClockDriver(config);
      auto ego_time = grape::EgoClock::TimePoint{};
      while (not st.stop_requested()) {
        const auto wall_time = grape::WallClock::now();
        driver.tick(ego_time, wall_time);
        std::this_thread::sleep_until(wall_time + WALL_TICK_PERIOD);
        ego_time += EGO_TICK_PERIOD;
      }
    } catch (...) {
      grape::Exception::print();
    }
  };

  auto master_clock_thread = std::jthread(master_clock, TEST_CLOCK_NAME);
  auto maybe_clock = grape::EgoClock::create(TEST_CLOCK_NAME, 2000ms);
  REQUIRE(maybe_clock);

  // NOLINTBEGIN(bugprone-unchecked-optional-access)

  // now() returns valid time when master is present
  const auto tp1 = maybe_clock->now();
  std::this_thread::sleep_for(5ms);
  const auto tp2 = maybe_clock->now();
  REQUIRE(tp2 > tp1);

  // sleepFor works correctly with master clock
  const auto sleep_duration = 20ms;
  const auto start_time = maybe_clock->now();
  maybe_clock->sleepFor(sleep_duration);
  const auto end_time = maybe_clock->now();

  // NOLINTEND(bugprone-unchecked-optional-access)

  const auto elapsed = end_time - start_time;
  const auto tolerance = 3ms;
  const auto min_expected = sleep_duration - tolerance;
  const auto max_expected = sleep_duration + tolerance;

  REQUIRE(elapsed >= min_expected);
  REQUIRE(elapsed <= max_expected);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock clock properties", "[ego_clock]") {
  REQUIRE_FALSE(grape::EgoClock::IS_STEADY);

  using Duration = grape::EgoClock::Duration;
  using TimePoint = grape::EgoClock::TimePoint;

  static_assert(std::is_same_v<Duration::period, std::nano>);
  static_assert(std::is_same_v<Duration::rep, std::int64_t>);

  static_assert(std::is_same_v<typename TimePoint::clock, grape::EgoClock>);
  static_assert(std::is_same_v<typename TimePoint::duration, Duration>);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
