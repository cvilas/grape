//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/clock/clock_broadcaster.h"
#include "grape/clock/follower_clock.h"
#include "grape/exception.h"
#include "grape/wall_clock.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Time conversion utilities", "[clock]") {
  SECTION("toNanos and fromNanos are inverse operations") {
    const auto nanos = 123456789LL;
    const auto tp = grape::clock::FollowerClock::fromNanos(nanos);
    REQUIRE(grape::clock::FollowerClock::toNanos(tp) == nanos);
  }

  SECTION("Zero nanoseconds maps to epoch") {
    const auto tp = grape::clock::FollowerClock::fromNanos(0);
    REQUIRE(grape::clock::FollowerClock::toNanos(tp) == 0);
  }

  SECTION("Negative nanoseconds are handled correctly") {
    const auto nanos = -123456789LL;
    const auto tp = grape::clock::FollowerClock::fromNanos(nanos);
    REQUIRE(grape::clock::FollowerClock::toNanos(tp) == nanos);
  }

  SECTION("Large nanosecond values are handled correctly") {
    const auto nanos = 9223372036854775LL;
    const auto tp = grape::clock::FollowerClock::fromNanos(nanos);
    REQUIRE(grape::clock::FollowerClock::toNanos(tp) == nanos);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("FollowerClock time point arithmetic", "[clock]") {
  SECTION("Time point addition with duration") {
    const auto tp1 = grape::clock::FollowerClock::fromNanos(1000000000LL);
    const auto duration = std::chrono::milliseconds(500);
    const auto tp2 = tp1 + duration;

    const auto expected_nanos = 1000000000LL + 500000000LL;
    REQUIRE(grape::clock::FollowerClock::toNanos(tp2) == expected_nanos);
  }

  SECTION("Duration between time points") {
    const auto tp1 = grape::clock::FollowerClock::fromNanos(1000000000LL);
    const auto tp2 = grape::clock::FollowerClock::fromNanos(2000000000LL);
    const auto duration = tp2 - tp1;

    const auto expected_nanos = 1000000000LL;
    REQUIRE(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() ==
            expected_nanos);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("FollowerClock clock properties", "[clock]") {
  REQUIRE_FALSE(grape::clock::FollowerClock::IS_STEADY);

  using Duration = grape::clock::FollowerClock::Duration;
  using TimePoint = grape::clock::FollowerClock::TimePoint;

  static_assert(std::is_same_v<Duration::period, std::nano>);
  static_assert(std::is_same_v<Duration::rep, std::int64_t>);

  static_assert(std::is_same_v<typename TimePoint::clock, grape::clock::FollowerClock>);
  static_assert(std::is_same_v<typename TimePoint::duration, Duration>);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("FollowerClock operation with broadcaster", "[clock]") {
  using namespace std::chrono_literals;

  static constexpr auto TICK_PERIOD = 10ms;
  const auto clock_name =
      std::format("test_clock_{}", grape::WallClock::now().time_since_epoch().count());

  // Create a driver in a background thread
  const auto driver_thread = [](const std::stop_token& st, const std::string& name) {
    try {
      const auto config = grape::clock::ClockBroadcaster::Config{ .name = name };
      auto driver = grape::clock::ClockBroadcaster(config);
      auto ego_time = grape::clock::FollowerClock::TimePoint{};
      while (not st.stop_requested()) {
        ego_time += TICK_PERIOD;
        driver.post(ego_time);
        std::this_thread::sleep_for(TICK_PERIOD);
      }
    } catch (...) {
      grape::Exception::print();
    }
  };

  auto driver = std::jthread(driver_thread, clock_name);

  auto clock = grape::clock::FollowerClock(clock_name);

  // NOLINTBEGIN(bugprone-unchecked-optional-access)

  // now() advances after sleepFor
  const auto tp1 = clock.now();
  clock.sleepFor(TICK_PERIOD);
  const auto tp2 = clock.now();
  REQUIRE(tp2 > tp1);

  // sleepFor blocks until sufficient ticks have elapsed
  static constexpr auto SLEEP_DURATION = 30ms;
  const auto before = clock.now();
  clock.sleepFor(SLEEP_DURATION);
  const auto after = clock.now();
  REQUIRE(after >= before + SLEEP_DURATION);

  // sleepUntil blocks until the target time is reached
  const auto target = clock.now() + SLEEP_DURATION;
  clock.sleepUntil(target);
  REQUIRE(clock.now() >= target);

  // NOLINTEND(bugprone-unchecked-optional-access)
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
