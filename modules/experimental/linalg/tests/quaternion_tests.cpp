//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cmath>
#include <compare>
#include <cstddef>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/linalg/quaternion.h"

namespace {

using Catch::Approx;
using grape::linalg::conjugate;
using grape::linalg::inverse;
using grape::linalg::norm;
using grape::linalg::normalized;
using grape::linalg::normSquared;
using grape::linalg::Quaterniond;
using grape::linalg::rotationMatrix;
using grape::linalg::trace;

constexpr auto QUATERNION_EPSILON = 1.0e-12;

void requireQuaternionApprox(const Quaterniond& actual, const Quaterniond& expected) {
  REQUIRE(actual.x == Approx(expected.x).epsilon(QUATERNION_EPSILON));
  REQUIRE(actual.y == Approx(expected.y).epsilon(QUATERNION_EPSILON));
  REQUIRE(actual.z == Approx(expected.z).epsilon(QUATERNION_EPSILON));
  REQUIRE(actual.w == Approx(expected.w).epsilon(QUATERNION_EPSILON));
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
// Quaternion algebra is usable in constant expressions (with both GCC and Clang). Operands are
// chosen so every intermediate is exact, making exact comparisons (std::is_eq) safe.
static_assert([] {
  constexpr auto UNIT_K = Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 };
  const auto kk = UNIT_K * UNIT_K;
  return std::is_eq(kk.x <=> 0.) && std::is_eq(kk.y <=> 0.) && std::is_eq(kk.z <=> 0.) &&
         std::is_eq(kk.w <=> -1.);
}());
static_assert(std::is_eq(normSquared(Quaterniond{ .x = 1, .y = 2, .z = 3, .w = 4 }) <=> 30.));
static_assert(std::is_eq(inverse(Quaterniond{ .x = 0, .y = 0, .z = 2, .w = 0 }).z <=> -0.5));
static_assert(rotationMatrix(Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 }) ==
              grape::linalg::Matrix<3, 3>{ -1., 0., 0., 0., -1., 0., 0., 0., 1. });

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion product composes rotations right-to-left, like rotation matrices",
          "[linalg]") {
  // C(lhs * rhs) == C(lhs) * C(rhs) (Trawny Eq. 63): rhs is applied first
  const auto lhs = Quaterniond::fromAxisAngle(grape::linalg::Vector<3>{ 1., 0., 0. }, 0.7);
  const auto rhs = Quaterniond::fromAxisAngle(grape::linalg::Vector<3>{ 0., 1., 0. }, -0.4);
  const auto composed = rotationMatrix(lhs * rhs);
  const auto expected = rotationMatrix(lhs) * rotationMatrix(rhs);
  for (std::size_t row = 0UZ; row < 3UZ; ++row) {
    for (std::size_t col = 0UZ; col < 3UZ; ++col) {
      REQUIRE(composed(row, col) == Approx(expected(row, col)).margin(QUATERNION_EPSILON));
    }
  }
  // non-commutative: the reversed order gives a different rotation
  const auto reversed = rotationMatrix(rhs * lhs);
  REQUIRE(reversed(0, 1) != Approx(expected(0, 1)).margin(1e-3));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion default construction provides the identity", "[linalg]") {
  const auto identity = Quaterniond{};
  requireQuaternionApprox(identity, Quaterniond{ .x = 0, .y = 0, .z = 0, .w = 1 });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion identity is neutral under multiplication", "[linalg]") {
  const auto quaternion = Quaterniond{ .x = 1, .y = 2, .z = 3, .w = 4 };
  const auto identity = Quaterniond{};

  requireQuaternionApprox(quaternion * identity, quaternion);
  requireQuaternionApprox(identity * quaternion, quaternion);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion multiplication follows Trawny Eq. 9 and Eq. 10", "[linalg]") {
  const auto quaternion = Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 };
  requireQuaternionApprox(quaternion * quaternion, Quaterniond{ .x = 0, .y = 0, .z = 0, .w = -1 });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion conjugation follows Trawny Eq. 21", "[linalg]") {
  const auto quaternion = Quaterniond{ .x = 1, .y = 2, .z = 3, .w = 4 };
  requireQuaternionApprox(conjugate(quaternion), Quaterniond{ .x = -1, .y = -2, .z = -3, .w = 4 });
  requireQuaternionApprox(conjugate(conjugate(quaternion)), quaternion);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion norm and normalization follow Trawny Eq. 5", "[linalg]") {
  const auto quaternion = Quaterniond{ .x = 0, .y = 0, .z = 2, .w = 0 };

  REQUIRE(normSquared(quaternion) == Approx(4.0));
  REQUIRE(norm(Quaterniond{}) == Approx(1.0));
  requireQuaternionApprox(normalized(quaternion), Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion inverse follows Trawny Eq. 22", "[linalg]") {
  const auto quaternion = Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 };
  const auto identity = Quaterniond{};

  requireQuaternionApprox(quaternion * inverse(quaternion), identity);
  requireQuaternionApprox(inverse(quaternion) * quaternion, identity);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion inverse reverses multiplication order", "[linalg]") {
  const auto lhs = Quaterniond{ .x = 1, .y = 0, .z = 0, .w = 1 };
  const auto rhs = Quaterniond{ .x = 0, .y = 1, .z = 0, .w = 1 };

  requireQuaternionApprox(inverse(lhs * rhs), inverse(rhs) * inverse(lhs));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Identity quaternion produces the identity rotation matrix", "[linalg]") {
  const auto matrix = rotationMatrix(Quaterniond{});

  REQUIRE(matrix(0, 0) == Approx(1.0));
  REQUIRE(matrix(0, 1) == Approx(0.0));
  REQUIRE(matrix(0, 2) == Approx(0.0));
  REQUIRE(matrix(1, 0) == Approx(0.0));
  REQUIRE(matrix(1, 1) == Approx(1.0));
  REQUIRE(matrix(1, 2) == Approx(0.0));
  REQUIRE(matrix(2, 0) == Approx(0.0));
  REQUIRE(matrix(2, 1) == Approx(0.0));
  REQUIRE(matrix(2, 2) == Approx(1.0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Half-turn quaternion produces the expected rotation matrix", "[linalg]") {
  const auto matrix = rotationMatrix(Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 });

  REQUIRE(matrix(0, 0) == Approx(-1.0));
  REQUIRE(matrix(0, 1) == Approx(0.0));
  REQUIRE(matrix(0, 2) == Approx(0.0));
  REQUIRE(matrix(1, 0) == Approx(0.0));
  REQUIRE(matrix(1, 1) == Approx(-1.0));
  REQUIRE(matrix(1, 2) == Approx(0.0));
  REQUIRE(matrix(2, 0) == Approx(0.0));
  REQUIRE(matrix(2, 1) == Approx(0.0));
  REQUIRE(matrix(2, 2) == Approx(1.0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Axis-angle quaternion follows Trawny Eq. 86", "[linalg]") {
  const auto angle = 0.8;
  const auto quat = Quaterniond::fromAxisAngle(grape::linalg::Vector<3>{ 0., 0., 1. }, angle);

  requireQuaternionApprox(
      quat, Quaterniond{ .x = 0., .y = 0., .z = std::sin(angle / 2), .w = std::cos(angle / 2) });
  REQUIRE(norm(quat) == Approx(1.0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion from rotation matrix inverts rotationMatrix", "[linalg]") {
  // a generic rotation (trace > 0 branch of Shepperd's method)
  const auto quat = normalized(Quaterniond{ .x = 0.1, .y = -0.2, .z = 0.3, .w = 0.9 });
  requireQuaternionApprox(Quaterniond::fromMatrix(rotationMatrix(quat)), quat);

  // half-turns about each axis exercise the remaining branches (trace = -1, w = 0)
  requireQuaternionApprox(
      Quaterniond::fromMatrix(rotationMatrix(Quaterniond{ .x = 1., .y = 0., .z = 0., .w = 0. })),
      Quaterniond{ .x = 1., .y = 0., .z = 0., .w = 0. });
  requireQuaternionApprox(
      Quaterniond::fromMatrix(rotationMatrix(Quaterniond{ .x = 0., .y = 1., .z = 0., .w = 0. })),
      Quaterniond{ .x = 0., .y = 1., .z = 0., .w = 0. });
  requireQuaternionApprox(
      Quaterniond::fromMatrix(rotationMatrix(Quaterniond{ .x = 0., .y = 0., .z = 1., .w = 0. })),
      Quaterniond{ .x = 0., .y = 0., .z = 1., .w = 0. });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion from rotation matrix pivots on the largest component", "[linalg]") {
  // General rotations whose largest component is x, y and z in turn (each made positive, since
  // the result has the sign of its largest component). The first two have a positive trace, so a
  // trace > 0 shortcut would pivot on the smaller w instead.
  const auto x_largest = normalized(Quaterniond{ .x = 0.75, .y = 0.2, .z = -0.1, .w = 0.6 });
  const auto y_largest = normalized(Quaterniond{ .x = -0.2, .y = 0.75, .z = 0.1, .w = 0.6 });
  const auto z_largest = normalized(Quaterniond{ .x = 0.1, .y = -0.4, .z = 0.9, .w = 0.15 });
  REQUIRE(trace(rotationMatrix(x_largest)) > 0.);
  REQUIRE(trace(rotationMatrix(y_largest)) > 0.);

  requireQuaternionApprox(Quaterniond::fromMatrix(rotationMatrix(x_largest)), x_largest);
  requireQuaternionApprox(Quaterniond::fromMatrix(rotationMatrix(y_largest)), y_largest);
  requireQuaternionApprox(Quaterniond::fromMatrix(rotationMatrix(z_largest)), z_largest);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
