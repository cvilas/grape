//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "../src/line_fitter.h"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(bugprone-unchecked-optional-access)

//-------------------------------------------------------------------------------------------------
TEST_CASE("LineFitter requires window size > 1", "[line_fitter]") {
  REQUIRE_THROWS_AS(grape::ego_clock::LineFitter(0), grape::Exception);
  REQUIRE_THROWS_AS(grape::ego_clock::LineFitter(1), grape::Exception);
  REQUIRE_NOTHROW(grape::ego_clock::LineFitter(2));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Fits line when sample set is complete", "[line_fitter]") {
  static constexpr auto WINDOW_SIZE = 4U;
  grape::ego_clock::LineFitter fitter(WINDOW_SIZE);

  fitter.add({ .x = 1.0, .y = 2.0 });
  REQUIRE_FALSE(fitter.fit());  // No fit result until minimum two samples
  fitter.add({ .x = 2.0, .y = 4.0 });
  REQUIRE(fitter.fit());
  fitter.add({ .x = 3.0, .y = 6.0 });
  REQUIRE(fitter.fit());
  fitter.add({ .x = 4.0, .y = 8.0 });
  REQUIRE(fitter.fit());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Perfect linear fit", "[line_fitter]") {
  static constexpr auto WINDOW_SIZE = 4U;
  grape::ego_clock::LineFitter fitter(WINDOW_SIZE);

  // y = 2x + 1
  fitter.add({ .x = 1.0, .y = 3.0 });  // 2*1 + 1 = 3
  fitter.add({ .x = 2.0, .y = 5.0 });  // 2*2 + 1 = 5
  fitter.add({ .x = 3.0, .y = 7.0 });  // 2*3 + 1 = 7
  fitter.add({ .x = 4.0, .y = 9.0 });  // 2*4 + 1 = 9

  auto result = fitter.fit();
  REQUIRE(result.has_value());
  REQUIRE(result->slope == Catch::Approx(2.0).margin(1e-10));
  REQUIRE(result->intercept == Catch::Approx(1.0).margin(1e-10));
  REQUIRE(result->mse == Catch::Approx(0.0).margin(1e-10));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Line fits with noisy data", "[line_fitter]") {
  static constexpr auto WINDOW_SIZE = 4U;
  grape::ego_clock::LineFitter fitter(WINDOW_SIZE);

  // y = 3x - 2 with some noise
  fitter.add({ .x = 1.0, .y = 0.9 });   // Should be 1.0
  fitter.add({ .x = 2.0, .y = 4.1 });   // Should be 4.0
  fitter.add({ .x = 3.0, .y = 6.8 });   // Should be 7.0
  fitter.add({ .x = 4.0, .y = 10.2 });  // Should be 10.0

  auto result = fitter.fit();
  REQUIRE(result.has_value());
  REQUIRE(result->slope == Catch::Approx(3.0).margin(0.1));
  REQUIRE(result->intercept == Catch::Approx(-2.0).margin(0.2));
  REQUIRE(result->mse > 0.0);  // There should be some error due to noise
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("LineFitter circular buffer behavior", "[line_fitter]") {
  static constexpr auto WINDOW_SIZE = 4U;
  grape::ego_clock::LineFitter fitter(WINDOW_SIZE);

  // Fill the buffer
  fitter.add({ .x = 1.0, .y = 1.0 });
  fitter.add({ .x = 2.0, .y = 2.0 });
  fitter.add({ .x = 3.0, .y = 3.0 });
  fitter.add({ .x = 4.0, .y = 4.0 });

  // First fit - should be y = x
  auto result1 = fitter.fit();
  REQUIRE(result1.has_value());
  REQUIRE(result1->slope == Catch::Approx(1.0).margin(1e-10));
  REQUIRE(result1->intercept == Catch::Approx(0.0).margin(1e-10));

  // Overwrite all points with a new line y = 2x
  fitter.add({ .x = 1.0, .y = 2.0 });
  fitter.add({ .x = 2.0, .y = 4.0 });
  fitter.add({ .x = 3.0, .y = 6.0 });
  fitter.add({ .x = 4.0, .y = 8.0 });

  // Second fit - should now be y = 2x
  auto result2 = fitter.fit();
  REQUIRE(result2.has_value());
  REQUIRE(result2->slope == Catch::Approx(2.0).margin(1e-10));
  REQUIRE(result2->intercept == Catch::Approx(0.0).margin(1e-10));

  // Partially overwrite buffer with points from y = 0.5x + 1
  fitter.add({ .x = 5.0, .y = 3.5 });  // 0.5*5 + 1 = 3.5
  fitter.add({ .x = 6.0, .y = 4.0 });  // 0.5*6 + 1 = 4.0

  // Third fit - should be a mix of old and new points
  // We expect 2 points from y = 2x and 2 points from y = 0.5x + 1
  auto result3 = fitter.fit();
  REQUIRE(result3.has_value());
  // The exact values would depend on which points are in the buffer
  // but we can check that the slope has changed from 2.0
  REQUIRE(result3->slope != Catch::Approx(2.0).margin(0.1));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("LineFitter with horizontal line", "[line_fitter]") {
  static constexpr auto WINDOW_SIZE = 4U;
  grape::ego_clock::LineFitter fitter(WINDOW_SIZE);

  // horizontal line: y = 5
  fitter.add({ .x = 1.0, .y = 5.0 });
  fitter.add({ .x = 2.0, .y = 5.0 });
  fitter.add({ .x = 3.0, .y = 5.0 });
  fitter.add({ .x = 4.0, .y = 5.0 });

  auto result = fitter.fit();
  REQUIRE(result.has_value());
  REQUIRE(result->slope == Catch::Approx(0.0).margin(1e-10));
  REQUIRE(result->intercept == Catch::Approx(5.0).margin(1e-10));
  REQUIRE(result->mse == Catch::Approx(0.0).margin(1e-10));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("LineFitter with vertical variation", "[line_fitter]") {
  static constexpr auto WINDOW_SIZE = 4U;
  grape::ego_clock::LineFitter fitter(WINDOW_SIZE);

  // vertical line: x = 5 with varying y
  fitter.add({ .x = 5.0, .y = 1.0 });
  fitter.add({ .x = 5.0, .y = 2.0 });
  fitter.add({ .x = 5.0, .y = 3.0 });
  fitter.add({ .x = 5.0, .y = 4.0 });

  // ill-conditioned problem: slope is theoretically infinite due to division by near-zero
  auto result = fitter.fit();
  REQUIRE(result.has_value());
  // No need to check exact values as they're numerically unstable.
  // But we should get here without crashing
}

// NOLINTEND(bugprone-unchecked-optional-access)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
