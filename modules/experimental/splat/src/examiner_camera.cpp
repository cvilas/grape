//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/splat/examiner_camera.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

using grape::splat::Mat3;
using grape::splat::Quat;
using grape::splat::Vec3;

constexpr auto PARALLEL_TOLERANCE = 1e-6F;
constexpr auto HALF = 0.5F;

//-------------------------------------------------------------------------------------------------
/// @return Camera-to-world rotation for a camera whose +Z axis points along `back`, with +Y axis
/// as close as possible to `up_hint`
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto lookRotation(const Vec3& back, const Vec3& up_hint) -> Quat {
  const auto bz = normalize(back);
  auto right = cross(up_hint, bz);
  if (length(right) < PARALLEL_TOLERANCE) {
    // up_hint is parallel to the viewing direction; choose any perpendicular
    right = cross((std::abs(bz.x) < 0.9F) ? Vec3{ .x = 1.F } : Vec3{ .y = 1.F }, bz);  // NOLINT
  }
  const auto bx = normalize(right);
  const auto by = cross(bz, bx);
  const auto mat = Mat3{ { { bx.x, by.x, bz.x }, { bx.y, by.y, bz.y }, { bx.z, by.z, bz.z } } };
  return grape::splat::fromMatrix(mat);
}

//-------------------------------------------------------------------------------------------------
/// Project normalised screen point onto the virtual trackball. A sphere near the centre, blended
/// into a hyperbolic sheet away from it so that rotation is continuous everywhere.
/// Ref: Bell's trackball, Gavin Bell, SGI, 1988
auto projectToTrackball(float px, float py) -> Vec3 {
  static constexpr auto RADIUS_SQ = 1.F;
  static constexpr auto HALF_RADIUS_SQ = RADIUS_SQ / 2.F;
  const auto d2 = (px * px) + (py * py);
  const auto pz =
      (d2 <= HALF_RADIUS_SQ) ? std::sqrt(RADIUS_SQ - d2) : HALF_RADIUS_SQ / std::sqrt(d2);
  return { .x = px, .y = py, .z = pz };
}

}  // namespace

namespace grape::splat {

//-------------------------------------------------------------------------------------------------
ExaminerCamera::ExaminerCamera(float fov_y) : fov_y_(fov_y) {
  if (fov_y <= 0.F or fov_y >= std::numbers::pi_v<float> or std::isnan(fov_y)) {
    fov_y_ = DEFAULT_FOV_Y;
  }
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::viewAll(const Vec3& center, float radius, const Vec3& up_hint) {
  const auto up = normalize(up_hint);
  // Prefer world +X as screen-right; the camera then looks perpendicular to it
  const auto right_hint = (std::abs(up.x) < 0.9F) ? Vec3{ .x = 1.F } : Vec3{ .y = 1.F };  // NOLINT
  orientation_ = lookRotation(cross(right_hint, up), up);
  focal_point_ = center;
  const auto safe_radius = std::max(radius, MIN_DISTANCE);
  distance_ = safe_radius / std::sin(HALF * fov_y_);
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto ExaminerCamera::trackball(float from_x, float from_y, float to_x, float to_y, float aspect)
    -> Quat {
  const auto p0 = normalize(projectToTrackball(from_x * aspect, from_y));
  const auto p1 = normalize(projectToTrackball(to_x * aspect, to_y));
  const auto axis = cross(p0, p1);
  const auto angle = std::atan2(length(axis), dot(p0, p1));
  const auto delta = fromAxisAngle(axis, angle);
  spin(delta);
  return delta;
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::spin(const Quat& camera_frame_rotation) {
  // Rotating the scene by q in camera frame is equivalent to rotating the camera by q⁻¹
  orientation_ = normalize(orientation_ * conjugate(camera_frame_rotation));
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void ExaminerCamera::pan(float dx_px, float dy_px, float viewport_height_px) {
  if (viewport_height_px <= 0.F) {
    return;
  }
  const auto metres_per_px = distance_ / focalLengthPx(viewport_height_px);
  const auto shift = Vec3{ .x = -dx_px * metres_per_px, .y = dy_px * metres_per_px, .z = 0.F };
  focal_point_ = focal_point_ + grape::splat::rotate(orientation_, shift);
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::dolly(float factor) {
  if (not(factor > 0.F)) {
    return;
  }
  distance_ = std::max(distance_ * factor, MIN_DISTANCE);
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::seek(const Vec3& point, float approach) {
  const auto back = position() - point;
  const auto dist = length(back);
  if (dist < MIN_DISTANCE) {
    return;
  }
  const auto up = grape::splat::rotate(orientation_, Vec3{ .y = 1.F });
  orientation_ = lookRotation(back, up);
  focal_point_ = point;
  distance_ = std::max(dist * std::clamp(approach, MIN_DISTANCE, 1.F), MIN_DISTANCE);
}

//-------------------------------------------------------------------------------------------------
auto ExaminerCamera::position() const -> Vec3 {
  return focal_point_ + grape::splat::rotate(orientation_, Vec3{ .z = distance_ });
}

//-------------------------------------------------------------------------------------------------
auto ExaminerCamera::viewMatrix() const -> Mat4 {
  // view = [Rᵀ | -Rᵀ·p], where R is camera-to-world rotation and p is camera position
  const auto rot = toMatrix(orientation_);
  const auto pos = position();
  auto view = Mat4{};
  for (auto row = 0UZ; row < 3; ++row) {
    for (auto col = 0UZ; col < 3; ++col) {
      view.m.at((col * 4) + row) = rot.at(col).at(row);
    }
    const auto axis =
        Vec3{ .x = rot.at(0).at(row), .y = rot.at(1).at(row), .z = rot.at(2).at(row) };
    view.m.at(12 + row) = -dot(axis, pos);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  }
  return view;
}

//-------------------------------------------------------------------------------------------------
auto ExaminerCamera::focalLengthPx(float viewport_height_px) const -> float {
  return HALF * viewport_height_px / std::tan(HALF * fov_y_);
}

}  // namespace grape::splat
