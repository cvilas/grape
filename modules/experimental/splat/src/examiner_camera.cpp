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
  const auto bz = grape::splat::normalize(back);
  auto right = cross(up_hint, bz);
  if (norm(right) < PARALLEL_TOLERANCE) {
    // up_hint is parallel to the viewing direction; choose any perpendicular
    right =
        cross((std::abs(bz[0]) < 0.9F) ? Vec3{ 1.F, 0.F, 0.F } : Vec3{ 0.F, 1.F, 0.F },  // NOLINT
              bz);
  }
  const auto bx = grape::splat::normalize(right);
  const auto by = cross(bz, bx);
  // The camera axes expressed in world coordinates are the columns of the camera-to-world matrix,
  // equivalently the rows of the world-to-camera matrix C(q) that Quaternion::fromMatrix() inverts
  const auto world_to_camera = Mat3{ bx[0], bx[1], bx[2],  //
                                     by[0], by[1], by[2],  //
                                     bz[0], bz[1], bz[2] };
  return grape::linalg::Quaternionf::fromMatrix(world_to_camera);
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
  return Vec3{ px, py, pz };
}

}  // namespace

namespace grape::splat {

//-------------------------------------------------------------------------------------------------
ExaminerCamera::ExaminerCamera(float fov_y) : fov_y_(fov_y) {
  if (fov_y < MIN_FOV_Y or fov_y > MAX_FOV_Y or std::isnan(fov_y)) {
    fov_y_ = DEFAULT_FOV_Y;
  }
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::viewAll(const Vec3& center, float radius, const Vec3& up_hint) {
  const auto up = grape::splat::normalize(up_hint);
  // Prefer world +X as screen-right; the camera then looks perpendicular to it
  const auto right_hint =
      (std::abs(up[0]) < 0.9F) ? Vec3{ 1.F, 0.F, 0.F } : Vec3{ 0.F, 1.F, 0.F };  // NOLINT
  orientation_ = lookRotation(cross(right_hint, up), up);
  focal_point_ = center;
  const auto safe_radius = std::max(radius, MIN_DISTANCE);
  distance_ = safe_radius / std::sin(HALF * fov_y_);
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto ExaminerCamera::trackball(float from_x, float from_y, float to_x, float to_y, float aspect)
    -> Quat {
  const auto p0 = grape::splat::normalize(projectToTrackball(from_x * aspect, from_y));
  const auto p1 = grape::splat::normalize(projectToTrackball(to_x * aspect, to_y));
  const auto axis = cross(p0, p1);
  const auto len = norm(axis);
  const auto angle = std::atan2(len, dot(p0, p1));
  // a degenerate axis (no pointer motion, or antipodal projections) means no rotation
  const auto delta =
      (len > 0.F) ? grape::linalg::Quaternionf::fromAxisAngle(axis * (1.F / len), angle) : Quat{};
  spin(delta);
  return delta;
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::spin(const Quat& camera_frame_rotation) {
  // Rotating the scene by q in camera frame is equivalent to rotating the camera by q⁻¹
  orientation_ = linalg::normalized(conjugate(camera_frame_rotation) * orientation_);
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void ExaminerCamera::pan(float dx_px, float dy_px, float viewport_height_px) {
  if (viewport_height_px <= 0.F) {
    return;
  }
  const auto metres_per_px = distance_ / focalLengthPx(viewport_height_px);
  const auto shift = Vec3{ -dx_px * metres_per_px, dy_px * metres_per_px, 0.F };
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
  const auto dist = norm(back);
  if (dist < MIN_DISTANCE) {
    return;
  }
  const auto up = grape::splat::rotate(orientation_, Vec3{ 0.F, 1.F, 0.F });
  orientation_ = lookRotation(back, up);
  focal_point_ = point;
  distance_ = std::max(dist * std::clamp(approach, MIN_DISTANCE, 1.F), MIN_DISTANCE);
}

//-------------------------------------------------------------------------------------------------
void ExaminerCamera::zoom(float factor) {
  if (factor <= 0.F or std::isnan(factor) or std::isinf(factor)) {
    return;
  }
  const auto fov_y = 2.F * std::atan(std::tan(HALF * fov_y_) * factor);
  fov_y_ = std::clamp(fov_y, MIN_FOV_Y, MAX_FOV_Y);
}

//-------------------------------------------------------------------------------------------------
auto ExaminerCamera::position() const -> Vec3 {
  return focal_point_ + grape::splat::rotate(orientation_, Vec3{ 0.F, 0.F, distance_ });
}

//-------------------------------------------------------------------------------------------------
auto ExaminerCamera::viewMatrix() const -> Mat4 {
  // view = [Rᵀ | -Rᵀ·p], where R is camera-to-world rotation and p is camera position. Rᵀ is the
  // world-to-camera rotation, i.e. linalg's passive rotationMatrix() of the orientation.
  const auto world_to_camera = linalg::rotationMatrix(orientation_);
  const auto pos = position();
  auto view = Mat4{};
  for (auto row = 0UZ; row < 3; ++row) {
    for (auto col = 0UZ; col < 3; ++col) {
      view.m.at((col * 4) + row) = world_to_camera[row, col];
    }
    const auto axis =
        Vec3{ world_to_camera[row, 0], world_to_camera[row, 1], world_to_camera[row, 2] };
    view.m.at(12 + row) = -dot(axis, pos);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  }
  return view;
}

//-------------------------------------------------------------------------------------------------
auto ExaminerCamera::focalLengthPx(float viewport_height_px) const -> float {
  return HALF * viewport_height_px / std::tan(HALF * fov_y_);
}

}  // namespace grape::splat
