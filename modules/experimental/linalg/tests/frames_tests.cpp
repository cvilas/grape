//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <concepts>
#include <cstddef>
#include <numbers>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/linalg/frames.h"

namespace {

using Catch::Approx;
using grape::linalg::Displacement;
using grape::linalg::Frame;
using grape::linalg::Matrix;
using grape::linalg::PointTag;
using grape::linalg::Position;
using grape::linalg::Quaterniond;
using grape::linalg::Rotation;
using grape::linalg::rotationMatrix;
using grape::linalg::rx;
using grape::linalg::ry;
using grape::linalg::rz;
using grape::linalg::Transform;
using grape::linalg::Vector;
using grape::linalg::Vector3;

// Example frames and points for an imaginary robot with a camera observing a landmark
struct World : Frame {};
struct Body : Frame {};
struct Camera : Frame {};
struct Landmark : PointTag {};

constexpr auto FRAMES_TEST_MARGIN = 1.0e-12;

void requireCoordsApprox(const Vector<3>& actual, const Vector<3>& expected) {
  for (std::size_t i = 0UZ; i < 3UZ; ++i) {
    REQUIRE(actual[i] == Approx(expected[i]).margin(FRAMES_TEST_MARGIN));
  }
}

void requireRotationsApprox(const Matrix<3, 3>& actual, const Matrix<3, 3>& expected) {
  for (std::size_t row = 0UZ; row < 3UZ; ++row) {
    for (std::size_t col = 0UZ; col < 3UZ; ++col) {
      REQUIRE(actual[row, col] == Approx(expected[row, col]).margin(FRAMES_TEST_MARGIN));
    }
  }
}

//-------------------------------------------------------------------------------------------------
// Mismatched frame semantics fail to compile. Each operation below is rejected by the type
// system; these checks are this module's core guarantee. (The probes must be concepts: an
// invalid expression makes a requires-clause false only during template substitution; written
// directly with concrete types it would be a hard compile error.)

template <typename Lhs, typename Rhs>
concept Addable = requires(const Lhs& lhs, const Rhs& rhs) { lhs + rhs; };

template <typename Lhs, typename Rhs>
concept Multipliable = requires(const Lhs& lhs, const Rhs& rhs) { lhs * rhs; };

// control: the probes recognise valid operations
static_assert(Addable<Vector3<World>, Vector3<World>>);
static_assert(Multipliable<Rotation<World, Body>, Rotation<Body, Camera>>);
static_assert(Multipliable<Transform<World, Body>, Position<Body, Landmark>>);

// vectors expressed in different bases cannot be added
static_assert(not Addable<Vector3<World>, Vector3<Body>>);
// broken frame chain: C_WB * C_CW
static_assert(not Multipliable<Rotation<World, Body>, Rotation<Camera, World>>);
// vector is already expressed in World; the rotation expects Body coordinates
static_assert(not Multipliable<Rotation<World, Body>, Vector3<World>>);
// the transform expects a Body-relative position
static_assert(not Multipliable<Transform<World, Body>, Position<Camera, Landmark>>);
// positions are anchored to an origin; adding them is meaningless
static_assert(not Addable<Position<World, Landmark>, Position<World, Landmark>>);
// a rotation alone cannot re-anchor a position; only a transform can
static_assert(not Multipliable<Rotation<World, Body>, Position<Body, Landmark>>);
// displacements only chain head-to-tail: r_AB + r_BC
static_assert(
    not Addable<Displacement<World, Body, Landmark>, Displacement<World, Body, Landmark>>);

// Raw coordinates acquire a frame only through an explicit constructor: a braced list cannot
// silently take on whatever frame a function parameter happens to declare
template <typename T>
concept ImplicitFromCoordinates = requires(void (*accept)(T)) { accept({ 0., 0., 0. }); };
static_assert(std::constructible_from<Vector3<World>, double, double, double>);
static_assert(not ImplicitFromCoordinates<Vector3<World>>);
static_assert(not ImplicitFromCoordinates<Displacement<World, Body, Landmark>>);
static_assert(not ImplicitFromCoordinates<Position<World, Landmark>>);

// Tagged types share a storage base, but one tag can never be constructed from another
static_assert(not std::constructible_from<Vector3<Body>, Vector3<World>>);
static_assert(not std::constructible_from<Displacement<World, Landmark, Body>,
                                          Displacement<World, Body, Landmark>>);
static_assert(not std::constructible_from<Position<Body, Landmark>, Position<World, Landmark>>);
static_assert(
    not std::constructible_from<Position<World, Landmark>, Displacement<World, World, Landmark>>);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
// The frame-aware layer is usable in constant expressions
static_assert((Vector3<World>{ 1., 2., 3. } + Vector3<World>{ 3., 2., 1. }).coordinates() ==
              Vector<3>{ 4., 4., 4. });
static_assert(dot(Vector3<World>{ 1., 0., 0. }, Vector3<World>{ 0., 1., 0. }) == 0.);
static_assert((Vector3<World>{ 1., 2., 3. } * 2).coordinates() == Vector<3>{ 2., 4., 6. });
static_assert((Vector3<World>{ 2., 4., 6. } / 2).coordinates() == Vector<3>{ 1., 2., 3. });

//-------------------------------------------------------------------------------------------------
TEST_CASE("Elementary rotations match the equivalent quaternion rotation matrix", "[linalg]") {
  // rx/ry/rz and rotationMatrix() share the same passive (frame-to-frame) convention
  const auto angle = 0.7;
  const auto half_sin = std::sin(angle / 2);
  const auto half_cos = std::cos(angle / 2);

  requireRotationsApprox(
      rx(angle), rotationMatrix(Quaterniond{ .x = half_sin, .y = 0., .z = 0., .w = half_cos }));
  requireRotationsApprox(
      ry(angle), rotationMatrix(Quaterniond{ .x = 0., .y = half_sin, .z = 0., .w = half_cos }));
  requireRotationsApprox(
      rz(angle), rotationMatrix(Quaterniond{ .x = 0., .y = 0., .z = half_sin, .w = half_cos }));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Rotation re-expresses a vector in the rotated frame", "[linalg]") {
  // Body is World rotated +90 degrees about Z. World's x-axis, seen from Body, points along -y.
  const auto rot = Rotation<Body, World>::aboutZ(std::numbers::pi / 2);
  const auto x_world = Vector3<World>{ 1., 0., 0. };
  requireCoordsApprox((rot * x_world).coordinates(), Vector<3>{ 0., -1., 0. });

  // The inverse rotation takes it back
  const auto roundtrip = rot.inverse() * (rot * x_world);
  requireCoordsApprox(roundtrip.coordinates(), x_world.coordinates());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Rotation composition follows the frame chain", "[linalg]") {
  const auto rot_wb = Rotation<World, Body>::aboutZ(0.3);
  const auto rot_bc = Rotation<Body, Camera>::aboutY(-0.2);
  const auto rot_wc = rot_wb * rot_bc;  // Rotation<World, Camera>
  requireRotationsApprox(rot_wc.matrix(), rot_wb.matrix() * rot_bc.matrix());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Rotation construction from a matrix is validated", "[linalg]") {
  REQUIRE(Rotation<World, Body>::fromMatrix(rz(0.5)).has_value());

  // not orthonormal
  const auto scaled = Matrix<3, 3>{ 2., 0., 0., 0., 2., 0., 0., 0., 2. };
  REQUIRE(!Rotation<World, Body>::fromMatrix(scaled).has_value());

  // orthonormal but a reflection (determinant -1)
  const auto reflection = Matrix<3, 3>{ 1., 0., 0., 0., 1., 0., 0., 0., -1. };
  REQUIRE(!Rotation<World, Body>::fromMatrix(reflection).has_value());

  // not finite
  const auto nan = std::numeric_limits<double>::quiet_NaN();
  const auto invalid = Matrix<3, 3>{ nan, 0., 0., 0., 1., 0., 0., 0., 1. };
  REQUIRE(!Rotation<World, Body>::fromMatrix(invalid).has_value());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Rotation from quaternion matches the elementary rotation", "[linalg]") {
  const auto angle = 1.2;
  const auto quat =
      Quaterniond{ .x = 0., .y = 0., .z = std::sin(angle / 2), .w = std::cos(angle / 2) };
  const auto from_quat = Rotation<Body, World>::fromQuaternion(quat);
  requireRotationsApprox(from_quat.matrix(), Rotation<Body, World>::aboutZ(angle).matrix());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Rotation from a non-unit quaternion is still a rotation", "[linalg]") {
  // rotationMatrix() of a quaternion with norm 2 would scale by up to 7; the Rotation invariant
  // (orthonormal, det +1) must hold regardless
  const auto angle = 1.2;
  const auto scaled =
      Quaterniond{ .x = 0., .y = 0., .z = 2. * std::sin(angle / 2), .w = 2. * std::cos(angle / 2) };
  const auto from_quat = Rotation<Body, World>::fromQuaternion(scaled);
  requireRotationsApprox(from_quat.matrix(), Rotation<Body, World>::aboutZ(angle).matrix());
  REQUIRE(Rotation<Body, World>::fromMatrix(from_quat.matrix()).has_value());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Displacements chain head-to-tail and reverse under negation", "[linalg]") {
  const auto wb = Displacement<World, World, Body>{ 1., 2., 0. };
  const auto bc = Displacement<World, Body, Camera>{ 0., 0., 3. };

  const auto wc = wb + bc;  // Displacement<World, World, Camera>
  requireCoordsApprox(wc.coordinates(), Vector<3>{ 1., 2., 3. });

  // -r_AB = r_BA
  requireCoordsApprox((-wb).coordinates(), Vector<3>{ -1., -2., 0. });

  // common-head subtraction: r_WC - r_BC = r_WB
  const auto recovered = wc - bc;
  requireCoordsApprox(recovered.coordinates(), wb.coordinates());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Positions difference to displacements and displace to positions", "[linalg]") {
  const auto p1 = Position<World, Body>{ 1., 0., 0. };
  const auto p2 = Position<World, Camera>{ 0., 1., 0. };

  const auto disp = p2 - p1;  // Displacement<World, Body, Camera>
  requireCoordsApprox(disp.coordinates(), Vector<3>{ -1., 1., 0. });
  requireCoordsApprox((p1 + disp).coordinates(), p2.coordinates());
  requireCoordsApprox((p2 - disp).coordinates(), p1.coordinates());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Transform maps positions between frames and composes along the chain", "[linalg]") {
  // Body is World yawed +90 degrees about z, with its origin at (1, 2, 0) in World. C_WB is the
  // inverse of the frame rotation that carries World into Body.
  const auto rot_wb = Rotation<Body, World>::aboutZ(std::numbers::pi / 2).inverse();
  const auto pose_wb =
      Transform<World, Body>{ rot_wb, Displacement<World, World, Body>{ 1., 2., 0. } };
  // Body-to-Camera: Camera 1m ahead of Body along its x-axis
  const auto pose_bc = Transform<Body, Camera>{ Rotation<Body, Camera>{},
                                                Displacement<Body, Body, Camera>{ 1., 0., 0. } };

  const auto pose_wc = pose_wb * pose_bc;  // Transform<World, Camera>

  // a landmark 1m ahead of the camera
  const auto p_camera = Position<Camera, Landmark>{ 1., 0., 0. };
  const auto p_world = pose_wc * p_camera;

  // World-to-Body rotation is +90 degrees about z (x_B seen in World is +y_W):
  // landmark = origin(1,2,0) + rotated(2,0,0) = (1, 4, 0)
  requireCoordsApprox(p_world.coordinates(), Vector<3>{ 1., 4., 0. });

  // the inverse transform takes it back
  const auto recovered = pose_wc.inverse() * p_world;
  requireCoordsApprox(recovered.coordinates(), p_camera.coordinates());

  // free vectors are re-expressed without translation
  const auto dir_camera = Vector3<Camera>{ 1., 0., 0. };
  const auto dir_world = pose_wc * dir_camera;
  requireCoordsApprox(dir_world.coordinates(), Vector<3>{ 0., 1., 0. });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Transform inverse reverses rotation and translation", "[linalg]") {
  const auto pose = Transform<World, Body>{ Rotation<World, Body>::aboutX(0.4),
                                            Displacement<World, World, Body>{ 1., -2., 3. } };
  const auto there_and_back = pose * pose.inverse();  // Transform<World, World>

  requireRotationsApprox(there_and_back.rotation().matrix(), grape::linalg::identity<3>());
  requireCoordsApprox(there_and_back.translation().coordinates(), Vector<3>{ 0., 0., 0. });
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
