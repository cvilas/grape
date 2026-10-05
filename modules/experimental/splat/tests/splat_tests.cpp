//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <numbers>
#include <vector>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/splat/math.h"
#include "grape/splat/splat.h"

namespace {

using Catch::Approx;
using grape::splat::ORIGIN;
using grape::splat::Quat;
using grape::splat::Splat;
using grape::splat::Vec3;

constexpr auto IDENTITY = Quat{};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
auto makeSplat(const Vec3& position, const Vec3& scale, const Quat& rotation) -> Splat {
  auto splat = Splat{};
  splat.position = position;
  splat.scale = scale;
  splat.rotation = rotation;
  return splat;
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Covariance of axis aligned splat is diagonal", "[splat]") {
  const auto splat = makeSplat(ORIGIN, { .x = 1.F, .y = 2.F, .z = 3.F }, IDENTITY);
  const auto cov = covariance(splat);
  CHECK(cov.at(0) == Approx(1.F));
  CHECK(cov.at(1) == Approx(0.F).margin(1e-6));
  CHECK(cov.at(2) == Approx(0.F).margin(1e-6));
  CHECK(cov.at(3) == Approx(4.F));
  CHECK(cov.at(4) == Approx(0.F).margin(1e-6));
  CHECK(cov.at(5) == Approx(9.F));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Covariance follows splat rotation", "[splat]") {
  // 90 deg about Z maps local X to world Y
  const auto rotation =
      grape::splat::fromAxisAngle(Vec3{ .z = 1.F }, 0.5F * std::numbers::pi_v<float>);
  const auto splat = makeSplat(ORIGIN, { .x = 1.F, .y = 2.F, .z = 3.F }, rotation);
  const auto cov = covariance(splat);
  CHECK(cov.at(0) == Approx(4.F));
  CHECK(cov.at(1) == Approx(0.F).margin(1e-5));
  CHECK(cov.at(3) == Approx(1.F));
  CHECK(cov.at(5) == Approx(9.F));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Scene bounds ignore far outliers", "[splat]") {
  auto splats = std::vector<Splat>{};
  for (auto i = -10; i <= 10; ++i) {
    splats.push_back(
        makeSplat({ .x = static_cast<float>(i) * 0.1F, .y = 5.F, .z = 0.F }, ORIGIN, IDENTITY));
  }
  splats.push_back(makeSplat({ .x = 1000.F, .y = 5.F, .z = 0.F }, ORIGIN, IDENTITY));
  const auto bounds = grape::splat::computeBounds(splats, 0.9F);
  CHECK(bounds.center.x == Approx(0.05F).margin(0.06));
  CHECK(bounds.center.y == Approx(5.F));
  CHECK(bounds.radius < 1.5F);
  CHECK(bounds.radius > 0.5F);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Scene bounds of empty scene are empty", "[splat]") {
  const auto bounds = grape::splat::computeBounds({});
  CHECK(bounds.radius == Approx(0.F));
  CHECK(bounds.center.x == Approx(0.F));
  CHECK(bounds.center.y == Approx(0.F));
  CHECK(bounds.center.z == Approx(0.F));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
