//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/statistics/sliding_mean.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
TEST_CASE("SlidingMean: Tests simple cases", "[statistics][sliding_mean]") {
  auto sliding_mean = grape::statistics::SlidingMean<double, 5>{};

  SECTION("First value") {
    auto stats = sliding_mean.append(42.0);
    REQUIRE(stats.mean == Catch::Approx(42.0));
    REQUIRE(stats.variance == Catch::Approx(0.0));  // Single value has zero variance
  }

  SECTION("Zero value") {
    auto stats = sliding_mean.append(0.0);
    REQUIRE(stats.mean == Catch::Approx(0.0));
    REQUIRE(stats.variance == Catch::Approx(0.0));
  }

  SECTION("Negative value") {
    auto stats = sliding_mean.append(-10.5);
    REQUIRE(stats.mean == Catch::Approx(-10.5));
    REQUIRE(stats.variance == Catch::Approx(0.0));
  }

  SECTION("Equal values") {
    std::ignore = sliding_mean.append(5.0);
    auto stats = sliding_mean.append(5.0);
    REQUIRE(stats.mean == Catch::Approx(5.0));
    REQUIRE(stats.variance == Catch::Approx(0.0));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SlidingMean: Tests sliding window behavior", "[statistics][sliding_mean]") {
  static constexpr auto EPSILON = 1e-6;
  auto sliding_mean = grape::statistics::SlidingMean<double, 3>{};

  auto stats1 = sliding_mean.append(1.0);
  REQUIRE(stats1.mean == Catch::Approx(1.0));
  REQUIRE(stats1.variance == Catch::Approx(0.0));

  auto stats2 = sliding_mean.append(2.0);
  REQUIRE(stats2.mean == Catch::Approx(1.5));
  REQUIRE(stats2.variance == Catch::Approx(0.5));

  auto stats3 = sliding_mean.append(5.0);
  REQUIRE(stats3.mean == Catch::Approx(2.666666).epsilon(EPSILON));
  REQUIRE(stats3.variance == Catch::Approx(4.333333).epsilon(EPSILON));

  auto stats4 = sliding_mean.append(8.0);
  REQUIRE(stats4.mean == Catch::Approx(5.0));
  REQUIRE(stats4.variance == Catch::Approx(9.0));

  auto stats5 = sliding_mean.append(9.0);
  REQUIRE(stats5.mean == Catch::Approx(7.333333).epsilon(EPSILON));
  REQUIRE(stats5.variance == Catch::Approx(4.333333).epsilon(EPSILON));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SlidingMean: Tests reset functionality", "[statistics][sliding_mean]") {
  auto sliding_mean = grape::statistics::SlidingMean<double, 3>{};

  SECTION("Reset after partial fill") {
    std::ignore = sliding_mean.append(1.0);
    const auto stats_before = sliding_mean.append(2.0);
    REQUIRE(stats_before.mean == Catch::Approx(1.5));

    sliding_mean.reset();

    const auto stats_after = sliding_mean.append(10.0);
    REQUIRE(stats_after.mean == Catch::Approx(10.0));
    REQUIRE(stats_after.variance == Catch::Approx(0.0));
  }

  SECTION("Reset after full window") {
    std::ignore = sliding_mean.append(1.0);
    std::ignore = sliding_mean.append(2.0);
    const auto stats_before = sliding_mean.append(3.0);
    REQUIRE(stats_before.mean == Catch::Approx(2.0));

    sliding_mean.reset();

    const auto stats_after = sliding_mean.append(100.0);
    REQUIRE(stats_after.mean == Catch::Approx(100.0));
    REQUIRE(stats_after.variance == Catch::Approx(0.0));
  }

  SECTION("Reset after sliding") {
    // Fill window and slide it
    std::ignore = sliding_mean.append(1.0);
    std::ignore = sliding_mean.append(2.0);
    std::ignore = sliding_mean.append(3.0);
    std::ignore = sliding_mean.append(4.0);              // Should slide to [2,3,4]
    const auto stats_before = sliding_mean.append(5.0);  // Should slide to [3,4,5]
    REQUIRE(stats_before.mean == Catch::Approx(4.0));

    sliding_mean.reset();

    const auto stats_after = sliding_mean.append(99.0);
    REQUIRE(stats_after.mean == Catch::Approx(99.0));
    REQUIRE(stats_after.variance == Catch::Approx(0.0));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SlidingMean: Tests numerical stability and edge cases", "[statistics][sliding_mean]") {
  auto sliding_mean = grape::statistics::SlidingMean<double, 3>{};

  SECTION("Very small values") {
    std::ignore = sliding_mean.append(1e-100);
    const auto stats = sliding_mean.append(2e-100);
    REQUIRE(stats.mean == Catch::Approx(1.5e-100));
    REQUIRE(stats.variance >= 0.0);  // Variance should be non-negative
  }

  SECTION("Very large values") {
    std::ignore = sliding_mean.append(1e100);
    const auto stats = sliding_mean.append(2e100);
    REQUIRE(stats.mean == Catch::Approx(1.5e100));
    REQUIRE(stats.variance >= 0.0);
  }

  SECTION("Mixed scale values") {
    std::ignore = sliding_mean.append(1e-6);
    std::ignore = sliding_mean.append(1.0);
    const auto stats = sliding_mean.append(1e6);
    const auto expected_mean = (1e-6 + 1.0 + 1e6) / 3.0;
    REQUIRE(stats.mean == Catch::Approx(expected_mean));
    REQUIRE(stats.variance >= 0.0);
  }

  SECTION("Welford's algorithm stability test") {
    const auto base = 1e12;
    std::ignore = sliding_mean.append(base + 1.0);
    std::ignore = sliding_mean.append(base + 2.0);
    const auto stats = sliding_mean.append(base + 3.0);
    REQUIRE(stats.mean == Catch::Approx(base + 2.0));
    REQUIRE(stats.variance == Catch::Approx(1.0));  // ((1-2)² + (2-2)² + (3-2)²) / 2 = 1
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
