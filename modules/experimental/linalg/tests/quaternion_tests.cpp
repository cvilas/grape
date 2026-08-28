//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cmath>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/linalg/quaternion.h"

namespace {

using Catch::Approx;
using grape::linalg::conjugate;
using grape::linalg::fromAxisAngle;
using grape::linalg::inverse;
using grape::linalg::norm;
using grape::linalg::normalized;
using grape::linalg::normSquared;
using grape::linalg::Quaterniond;
using grape::linalg::rotationMatrix;
using grape::linalg::toQuaternion;

constexpr auto QUATERNION_EPSILON = 1.0e-12;

void requireQuaternionApprox(const Quaterniond& actual, const Quaterniond& expected) {
  REQUIRE(actual.x == Approx(expected.x).epsilon(QUATERNION_EPSILON));
  REQUIRE(actual.y == Approx(expected.y).epsilon(QUATERNION_EPSILON));
  REQUIRE(actual.z == Approx(expected.z).epsilon(QUATERNION_EPSILON));
  REQUIRE(actual.w == Approx(expected.w).epsilon(QUATERNION_EPSILON));
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

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

  REQUIRE(matrix[0, 0] == Approx(1.0));
  REQUIRE(matrix[0, 1] == Approx(0.0));
  REQUIRE(matrix[0, 2] == Approx(0.0));
  REQUIRE(matrix[1, 0] == Approx(0.0));
  REQUIRE(matrix[1, 1] == Approx(1.0));
  REQUIRE(matrix[1, 2] == Approx(0.0));
  REQUIRE(matrix[2, 0] == Approx(0.0));
  REQUIRE(matrix[2, 1] == Approx(0.0));
  REQUIRE(matrix[2, 2] == Approx(1.0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Half-turn quaternion produces the expected rotation matrix", "[linalg]") {
  const auto matrix = rotationMatrix(Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 });

  REQUIRE(matrix[0, 0] == Approx(-1.0));
  REQUIRE(matrix[0, 1] == Approx(0.0));
  REQUIRE(matrix[0, 2] == Approx(0.0));
  REQUIRE(matrix[1, 0] == Approx(0.0));
  REQUIRE(matrix[1, 1] == Approx(-1.0));
  REQUIRE(matrix[1, 2] == Approx(0.0));
  REQUIRE(matrix[2, 0] == Approx(0.0));
  REQUIRE(matrix[2, 1] == Approx(0.0));
  REQUIRE(matrix[2, 2] == Approx(1.0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Axis-angle quaternion follows Trawny Eq. 86", "[linalg]") {
  const auto angle = 0.8;
  const auto quat = fromAxisAngle(grape::linalg::Vector<3>{ 0., 0., 1. }, angle);

  requireQuaternionApprox(
      quat, Quaterniond{ .x = 0., .y = 0., .z = std::sin(angle / 2), .w = std::cos(angle / 2) });
  REQUIRE(norm(quat) == Approx(1.0));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Quaternion from rotation matrix inverts rotationMatrix", "[linalg]") {
  // a generic rotation (trace > 0 branch of Shepperd's method)
  const auto quat = normalized(Quaterniond{ .x = 0.1, .y = -0.2, .z = 0.3, .w = 0.9 });
  requireQuaternionApprox(toQuaternion(rotationMatrix(quat)), quat);

  // half-turns about each axis exercise the remaining branches (trace = -1, w = 0)
  requireQuaternionApprox(
      toQuaternion(rotationMatrix(Quaterniond{ .x = 1., .y = 0., .z = 0., .w = 0. })),
      Quaterniond{ .x = 1., .y = 0., .z = 0., .w = 0. });
  requireQuaternionApprox(
      toQuaternion(rotationMatrix(Quaterniond{ .x = 0., .y = 1., .z = 0., .w = 0. })),
      Quaterniond{ .x = 0., .y = 1., .z = 0., .w = 0. });
  requireQuaternionApprox(
      toQuaternion(rotationMatrix(Quaterniond{ .x = 0., .y = 0., .z = 1., .w = 0. })),
      Quaterniond{ .x = 0., .y = 0., .z = 1., .w = 0. });
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
