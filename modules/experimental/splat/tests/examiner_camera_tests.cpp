//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cmath>
#include <numbers>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/splat/examiner_camera.h"
#include "grape/splat/math.h"

namespace {

using Catch::Approx;
using grape::splat::ExaminerCamera;
using grape::splat::ORIGIN;
using grape::splat::Vec3;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
void checkNear(const Vec3& actual, const Vec3& expected, float tol = 1e-4F) {
  CHECK(actual.x == Approx(expected.x).margin(tol));
  CHECK(actual.y == Approx(expected.y).margin(tol));
  CHECK(actual.z == Approx(expected.z).margin(tol));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("View all frames the sphere with the requested up direction", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  const auto center = Vec3{ .x = 1.F, .y = 2.F, .z = 3.F };
  constexpr auto RADIUS = 2.F;
  camera.viewAll(center, RADIUS, { .y = 1.F });

  checkNear(camera.focalPoint(), center);
  CHECK(camera.distance() == Approx(RADIUS / std::sin(0.5F * camera.fovY())));
  // y-up: camera looks along -Z from +Z
  checkNear(camera.position(), center + Vec3{ .z = camera.distance() });
  // focal point is straight ahead in view frame
  checkNear(transformPoint(camera.viewMatrix(), center), { .z = -camera.distance() });
  // world up appears as screen up
  checkNear(transformPoint(camera.viewMatrix(), center + Vec3{ .y = 1.F }),
            { .y = 1.F, .z = -camera.distance() });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("View all supports y-down and z-up conventions", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  for (const auto& up : { Vec3{ .y = -1.F }, Vec3{ .z = 1.F }, Vec3{ .x = 1.F } }) {
    camera.viewAll(ORIGIN, 1.F, up);
    const auto pt = transformPoint(camera.viewMatrix(), up);
    checkNear(pt, { .y = 1.F, .z = -camera.distance() });
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Trackball rotation makes the scene follow the pointer", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  camera.viewAll(ORIGIN, 1.F, { .y = 1.F });
  const auto distance = camera.distance();
  const auto front = Vec3{ .z = 1.F };  // point on scene facing the camera

  std::ignore = camera.trackball(0.F, 0.F, 0.2F, 0.F, 1.F);  // drag right
  const auto pt = transformPoint(camera.viewMatrix(), front);
  CHECK(pt.x > 0.F);
  CHECK(pt.y == Approx(0.F).margin(1e-5));
  // orbits the focal point at constant distance
  CHECK(length(camera.position()) == Approx(distance));
  checkNear(camera.focalPoint(), {});
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Trackball drag up tilts scene upwards", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  camera.viewAll(ORIGIN, 1.F, { .y = 1.F });
  std::ignore = camera.trackball(0.F, 0.F, 0.F, 0.3F, 1.F);
  const auto pt = transformPoint(camera.viewMatrix(), { .z = 1.F });
  CHECK(pt.y > 0.F);
  CHECK(pt.x == Approx(0.F).margin(1e-5));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Pan moves the focal plane with the pointer", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  camera.viewAll(ORIGIN, 1.F, { .y = 1.F });
  constexpr auto HEIGHT_PX = 600.F;
  constexpr auto DX_PX = 30.F;
  constexpr auto DY_PX = -20.F;  // up
  const auto focal_px = camera.focalLengthPx(HEIGHT_PX);
  camera.pan(DX_PX, DY_PX, HEIGHT_PX);

  // The original focal point should now project DX right and |DY| up of the image centre
  const auto pt = transformPoint(camera.viewMatrix(), {});
  const auto depth = -pt.z;
  CHECK(focal_px * pt.x / depth == Approx(DX_PX));
  CHECK(focal_px * pt.y / depth == Approx(-DY_PX));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Dolly scales distance to focal point", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  camera.viewAll(ORIGIN, 1.F, { .y = 1.F });
  const auto distance = camera.distance();
  camera.dolly(0.5F);
  CHECK(camera.distance() == Approx(0.5F * distance));
  camera.dolly(0.F);   // ignored
  camera.dolly(-1.F);  // ignored
  CHECK(camera.distance() == Approx(0.5F * distance));
  camera.dolly(1e-12F);
  CHECK(camera.distance() == Approx(ExaminerCamera::MIN_DISTANCE));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Seek aims at the point and moves towards it", "[examiner_camera]") {
  auto camera = ExaminerCamera{};
  camera.viewAll(ORIGIN, 1.F, { .y = 1.F });
  const auto target = Vec3{ .x = 0.5F, .y = 0.2F, .z = 0.F };
  const auto start = camera.position();
  camera.seek(target, 0.5F);

  checkNear(camera.focalPoint(), target);
  CHECK(camera.distance() == Approx(0.5F * length(start - target)));
  checkNear(transformPoint(camera.viewMatrix(), target), { .z = -camera.distance() });
  // camera moved along the line towards the target
  const auto dir_before = normalize(start - target);
  const auto dir_after = normalize(camera.position() - target);
  checkNear(dir_after, dir_before);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Invalid field of view falls back to default", "[examiner_camera]") {
  CHECK(ExaminerCamera(0.F).fovY() == Approx(ExaminerCamera::DEFAULT_FOV_Y));
  CHECK(ExaminerCamera(4.F).fovY() == Approx(ExaminerCamera::DEFAULT_FOV_Y));
  CHECK(ExaminerCamera(1.F).fovY() == Approx(1.F));
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
