//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/ego_clock2.h"
#include "grape/ego_clock2_driver.h"
#include "grape/exception.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock2 time conversion utilities", "[ego_clock2]") {
  SECTION("toNanos and fromNanos are inverse operations") {
    const auto nanos = 123456789LL;
    const auto tp = grape::EgoClock2::fromNanos(nanos);
    REQUIRE(grape::EgoClock2::toNanos(tp) == nanos);
  }

  SECTION("Zero nanoseconds maps to epoch") {
    const auto tp = grape::EgoClock2::fromNanos(0);
    REQUIRE(grape::EgoClock2::toNanos(tp) == 0);
  }

  SECTION("Negative nanoseconds are handled correctly") {
    const auto nanos = -123456789LL;
    const auto tp = grape::EgoClock2::fromNanos(nanos);
    REQUIRE(grape::EgoClock2::toNanos(tp) == nanos);
  }

  SECTION("Large nanosecond values are handled correctly") {
    const auto nanos = 9223372036854775LL;
    const auto tp = grape::EgoClock2::fromNanos(nanos);
    REQUIRE(grape::EgoClock2::toNanos(tp) == nanos);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock2 time point arithmetic", "[ego_clock2]") {
  SECTION("Time point addition with duration") {
    const auto tp1 = grape::EgoClock2::fromNanos(1000000000LL);
    const auto duration = std::chrono::milliseconds(500);
    const auto tp2 = tp1 + duration;

    const auto expected_nanos = 1000000000LL + 500000000LL;
    REQUIRE(grape::EgoClock2::toNanos(tp2) == expected_nanos);
  }

  SECTION("Duration between time points") {
    const auto tp1 = grape::EgoClock2::fromNanos(1000000000LL);
    const auto tp2 = grape::EgoClock2::fromNanos(2000000000LL);
    const auto duration = tp2 - tp1;

    const auto expected_nanos = 1000000000LL;
    REQUIRE(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() ==
            expected_nanos);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock2 clock properties", "[ego_clock2]") {
  REQUIRE_FALSE(grape::EgoClock2::IS_STEADY);

  using Duration = grape::EgoClock2::Duration;
  using TimePoint = grape::EgoClock2::TimePoint;

  static_assert(std::is_same_v<Duration::period, std::nano>);
  static_assert(std::is_same_v<Duration::rep, std::int64_t>);

  static_assert(std::is_same_v<typename TimePoint::clock, grape::EgoClock2>);
  static_assert(std::is_same_v<typename TimePoint::duration, Duration>);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("EgoClock2 operation with driver", "[ego_clock2]") {
  using namespace std::chrono_literals;

  static constexpr auto CLOCK_NAME = "/grape_test_ego_clock2";
  static constexpr auto TICK_PERIOD = 10ms;

  // Without driver, create() should time out
  REQUIRE_FALSE(grape::EgoClock2::create(CLOCK_NAME, 10ms));

  // Create a driver in a background thread
  const auto driver_thread = [](const std::stop_token& st, const std::string& clock_name) {
    try {
      const auto config = grape::EgoClock2Driver::Config{ .clock_name = clock_name };
      auto driver = grape::EgoClock2Driver(config);
      auto ego_time = grape::EgoClock2::TimePoint{};
      while (not st.stop_requested()) {
        ego_time += TICK_PERIOD;
        driver.tick(ego_time);
        std::this_thread::sleep_for(TICK_PERIOD);
      }
    } catch (...) {
      grape::Exception::print();
    }
  };

  auto driver = std::jthread(driver_thread, CLOCK_NAME);

  // Create a consumer clock and wait for first tick
  auto maybe_clock = grape::EgoClock2::create(CLOCK_NAME, 2000ms);
  REQUIRE(maybe_clock);

  // NOLINTBEGIN(bugprone-unchecked-optional-access)

  // now() returns valid non-zero time when driver is present
  const auto tp1 = maybe_clock->now();
  REQUIRE(grape::EgoClock2::toNanos(tp1) > 0);

  // now() advances after sleepFor
  maybe_clock->sleepFor(TICK_PERIOD);
  const auto tp2 = maybe_clock->now();
  REQUIRE(tp2 > tp1);

  // sleepFor blocks until sufficient ticks have elapsed
  static constexpr auto SLEEP_DURATION = 30ms;
  const auto before = maybe_clock->now();
  maybe_clock->sleepFor(SLEEP_DURATION);
  const auto after = maybe_clock->now();
  REQUIRE(after >= before + SLEEP_DURATION);

  // sleepUntil blocks until the target time is reached
  const auto target = maybe_clock->now() + SLEEP_DURATION;
  maybe_clock->sleepUntil(target);
  REQUIRE(maybe_clock->now() >= target);

  // NOLINTEND(bugprone-unchecked-optional-access)
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
