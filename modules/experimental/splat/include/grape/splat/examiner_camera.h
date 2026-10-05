//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <numbers>

#include "grape/splat/math.h"

namespace grape::splat {

//=================================================================================================
/// Camera that orbits a focal point, modelled on the OpenInventor examiner viewer.
///
/// Conventions:
/// - World and camera frames are right handed. The camera looks along its local -Z axis, with +Y
///   up and +X right (OpenGL/OpenInventor convention).
/// - The camera is located at `distance` from the focal point, along the camera +Z axis.
/// - Screen points are normalised: (-1, -1) is bottom-left and (1, 1) is top-right of viewport.
class ExaminerCamera {
public:
  static constexpr auto DEFAULT_FOV_Y = 50.F * std::numbers::pi_v<float> / 180.F;
  static constexpr auto MIN_DISTANCE = 1e-4F;

  /// @param fov_y Vertical field of view (radians, in (0, π))
  explicit ExaminerCamera(float fov_y = DEFAULT_FOV_Y);

  /// Place the camera so that the sphere fills the view. The camera looks along -up_hint x
  /// (an arbitrary perpendicular) towards the centre, with up_hint as screen-up.
  /// @param center Centre of sphere to view
  /// @param radius Radius of sphere to view (> 0)
  /// @param up_hint World direction to show as screen-up (non-zero)
  void viewAll(const Vec3& center, float radius, const Vec3& up_hint);

  /// Virtual trackball rotation about the focal point. The scene follows the pointer as it moves
  /// from `from` to `to`.
  /// @param from_x Previous pointer x position (normalised screen coordinates)
  /// @param from_y Previous pointer y position (normalised screen coordinates)
  /// @param to_x Current pointer x position (normalised screen coordinates)
  /// @param to_y Current pointer y position (normalised screen coordinates)
  /// @param aspect Viewport width / height
  /// @return Incremental rotation applied, in camera frame (Useful to keep the scene spinning)
  auto trackball(float from_x, float from_y, float to_x, float to_y, float aspect) -> Quat;

  /// Apply a rotation, expressed in camera frame, to the scene about the focal point
  void spin(const Quat& camera_frame_rotation);

  /// Translate the camera and focal point parallel to the view plane, such that a point at the
  /// focal distance follows the pointer.
  /// @param dx_px Horizontal pointer motion (pixels, +ve right)
  /// @param dy_px Vertical pointer motion (pixels, +ve down)
  /// @param viewport_height_px Viewport height (pixels)
  void pan(float dx_px, float dy_px, float viewport_height_px);

  /// Move the camera towards (factor < 1) or away from (factor > 1) the focal point
  /// @param factor Multiplier for distance to the focal point (> 0)
  void dolly(float factor);

  /// Aim the camera at a new focal point, and move towards it.
  /// @param point New focal point
  /// @param approach Fraction of the distance to the point to retain, in (0, 1]
  void seek(const Vec3& point, float approach);

  [[nodiscard]] auto focalPoint() const -> Vec3 {
    return focal_point_;
  }

  [[nodiscard]] auto distance() const -> float {
    return distance_;
  }

  [[nodiscard]] auto orientation() const -> Quat {
    return orientation_;
  }

  [[nodiscard]] auto fovY() const -> float {
    return fov_y_;
  }

  /// @return Camera position in world frame
  [[nodiscard]] auto position() const -> Vec3;

  /// @return World-to-camera transform
  [[nodiscard]] auto viewMatrix() const -> Mat4;

  /// @return Focal length in pixels for the given viewport height
  [[nodiscard]] auto focalLengthPx(float viewport_height_px) const -> float;

private:
  Vec3 focal_point_;
  Quat orientation_;  //!< camera-to-world rotation
  float distance_{ 1.F };
  float fov_y_{ DEFAULT_FOV_Y };
};

}  // namespace grape::splat
