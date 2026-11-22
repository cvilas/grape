//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <cstddef>

#include "grape/linalg/matrix.h"
#include "grape/linalg/quaternion.h"

namespace grape::splat {

//=================================================================================================
// Rendering math, built on grape::linalg. This header only adds what is specific to this viewer:
// rendering-friendly fallbacks for degenerate inputs, the 'active' rotation view of a quaternion,
// and the column-major 4x4 layout that GPUs expect.
//=================================================================================================

/// 3D vector. Units are context dependent (typically metres).
using Vec3 = linalg::Vector<3, float>;

/// Rotation quaternion, q = [x, y, z, w] with w the scalar part
using Quat = linalg::Quaternion<float>;

/// Row-major 3x3 matrix
using Mat3 = linalg::Matrix<3, 3, float>;

/// The zero vector
inline constexpr auto ORIGIN = Vec3{};

/// @return Unit vector along vec, or vec unchanged if it has zero length
[[nodiscard]] inline auto normalize(const Vec3& vec) -> Vec3 {
  const auto len = linalg::norm(vec);
  return (len > 0.F) ? vec * (1.F / len) : vec;
}

/// @return Unit quaternion, or identity if quat has zero norm (quantised file data can decode to
/// all-zero)
[[nodiscard]] inline auto normalize(const Quat& quat) -> Quat {
  return (linalg::normSquared(quat) > 0.F) ? linalg::normalized(quat) : Quat{};
}

/// @return Matrix that rotates vectors by quat within a fixed frame (the 'active' rotation of
/// computer graphics). linalg::rotationMatrix() answers the passive question - re-expressing
/// unmoved vectors in a rotated frame - and the two are transposes of each other.
[[nodiscard]] constexpr auto rotationOf(const Quat& quat) -> Mat3 {
  return transpose(linalg::rotationMatrix(quat));
}

/// Rotate vector by unit quaternion (active rotation: the vector turns, the frame stays)
[[nodiscard]] constexpr auto rotate(const Quat& quat, const Vec3& vec) -> Vec3 {
  return rotationOf(quat) * vec;
}

//=================================================================================================
/// 4x4 affine transform, stored column-major so it can be uploaded to shaders directly. This is a
/// GPU interchange format, not a math type; math happens in linalg types and lands here last.
struct Mat4 {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  std::array<float, 16> m{
    1.F, 0.F, 0.F, 0.F,  //
    0.F, 1.F, 0.F, 0.F,  //
    0.F, 0.F, 1.F, 0.F,  //
    0.F, 0.F, 0.F, 1.F,
  };

  /// @return element at specified row and column
  [[nodiscard]] constexpr auto at(std::size_t row, std::size_t col) const -> float {
    return m.at((col * 4U) + row);
  }

  constexpr auto operator==(const Mat4&) const -> bool = default;
};

/// Transform a point by an affine matrix
[[nodiscard]] constexpr auto transformPoint(const Mat4& tf, const Vec3& pt) -> Vec3 {
  return Vec3{
    (tf.at(0, 0) * pt(0)) + (tf.at(0, 1) * pt(1)) + (tf.at(0, 2) * pt(2)) + tf.at(0, 3),
    (tf.at(1, 0) * pt(0)) + (tf.at(1, 1) * pt(1)) + (tf.at(1, 2) * pt(2)) + tf.at(1, 3),
    (tf.at(2, 0) * pt(0)) + (tf.at(2, 1) * pt(1)) + (tf.at(2, 2) * pt(2)) + tf.at(2, 3),
  };
}

}  // namespace grape::splat
