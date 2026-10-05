//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <cmath>
#include <cstddef>

namespace grape::splat {

//=================================================================================================
/// Minimal 3D vector for rendering math. Units are context dependent (typically metres).
struct Vec3 {
  float x{ 0.F };
  float y{ 0.F };
  float z{ 0.F };
};

/// The zero vector
inline constexpr auto ORIGIN = Vec3{};

constexpr auto operator+(const Vec3& lhs, const Vec3& rhs) -> Vec3 {
  return { .x = lhs.x + rhs.x, .y = lhs.y + rhs.y, .z = lhs.z + rhs.z };
}

constexpr auto operator-(const Vec3& lhs, const Vec3& rhs) -> Vec3 {
  return { .x = lhs.x - rhs.x, .y = lhs.y - rhs.y, .z = lhs.z - rhs.z };
}

constexpr auto operator-(const Vec3& vec) -> Vec3 {
  return { .x = -vec.x, .y = -vec.y, .z = -vec.z };
}

constexpr auto operator*(const Vec3& vec, float scalar) -> Vec3 {
  return { .x = vec.x * scalar, .y = vec.y * scalar, .z = vec.z * scalar };
}

constexpr auto operator*(float scalar, const Vec3& vec) -> Vec3 {
  return vec * scalar;
}

constexpr auto dot(const Vec3& lhs, const Vec3& rhs) -> float {
  return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

constexpr auto cross(const Vec3& lhs, const Vec3& rhs) -> Vec3 {
  return {
    .x = (lhs.y * rhs.z) - (lhs.z * rhs.y),
    .y = (lhs.z * rhs.x) - (lhs.x * rhs.z),
    .z = (lhs.x * rhs.y) - (lhs.y * rhs.x),
  };
}

inline auto length(const Vec3& vec) -> float {
  return std::sqrt(dot(vec, vec));
}

/// @return Unit vector along vec, or vec unchanged if it has zero length
inline auto normalize(const Vec3& vec) -> Vec3 {
  const auto len = length(vec);
  return (len > 0.F) ? vec * (1.F / len) : vec;
}

//=================================================================================================
/// Rotation quaternion (Hamilton convention, w is the scalar part)
struct Quat {
  float w{ 1.F };
  float x{ 0.F };
  float y{ 0.F };
  float z{ 0.F };
};

/// Hamilton product. (lhs * rhs) applies rhs first, then lhs.
constexpr auto operator*(const Quat& lhs, const Quat& rhs) -> Quat {
  return {
    .w = (lhs.w * rhs.w) - (lhs.x * rhs.x) - (lhs.y * rhs.y) - (lhs.z * rhs.z),
    .x = (lhs.w * rhs.x) + (lhs.x * rhs.w) + (lhs.y * rhs.z) - (lhs.z * rhs.y),
    .y = (lhs.w * rhs.y) - (lhs.x * rhs.z) + (lhs.y * rhs.w) + (lhs.z * rhs.x),
    .z = (lhs.w * rhs.z) + (lhs.x * rhs.y) - (lhs.y * rhs.x) + (lhs.z * rhs.w),
  };
}

/// @return Inverse of a unit quaternion
constexpr auto conjugate(const Quat& quat) -> Quat {
  return { .w = quat.w, .x = -quat.x, .y = -quat.y, .z = -quat.z };
}

/// @return Unit quaternion, or identity if quat has zero norm
inline auto normalize(const Quat& quat) -> Quat {
  const auto norm =
      std::sqrt((quat.w * quat.w) + (quat.x * quat.x) + (quat.y * quat.y) + (quat.z * quat.z));
  if (norm <= 0.F) {
    return {};
  }
  const auto inv = 1.F / norm;
  return { .w = quat.w * inv, .x = quat.x * inv, .y = quat.y * inv, .z = quat.z * inv };
}

/// @param axis Rotation axis (need not be normalised; zero axis yields identity)
/// @param angle Rotation angle (radians)
inline auto fromAxisAngle(const Vec3& axis, float angle) -> Quat {
  const auto len = length(axis);
  if (len <= 0.F) {
    return {};
  }
  const auto half_angle = angle / 2.F;
  const auto scale = std::sin(half_angle) / len;
  return {
    .w = std::cos(half_angle),
    .x = axis.x * scale,
    .y = axis.y * scale,
    .z = axis.z * scale,
  };
}

/// Rotate vector by unit quaternion
constexpr auto rotate(const Quat& quat, const Vec3& vec) -> Vec3 {
  // v' = v + 2w(u x v) + 2u x (u x v), where u = (x, y, z)
  const auto axis = Vec3{ .x = quat.x, .y = quat.y, .z = quat.z };
  const auto tmp = 2.F * cross(axis, vec);
  return vec + (quat.w * tmp) + cross(axis, tmp);
}

//=================================================================================================
/// Row-major 3x3 matrix
using Mat3 = std::array<std::array<float, 3>, 3>;

/// @return Rotation matrix equivalent of unit quaternion
constexpr auto toMatrix(const Quat& quat) -> Mat3 {
  const auto xx = quat.x * quat.x;
  const auto yy = quat.y * quat.y;
  const auto zz = quat.z * quat.z;
  const auto xy = quat.x * quat.y;
  const auto xz = quat.x * quat.z;
  const auto yz = quat.y * quat.z;
  const auto wx = quat.w * quat.x;
  const auto wy = quat.w * quat.y;
  const auto wz = quat.w * quat.z;
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  auto mat = Mat3{};
  mat.at(0) = { 1.F - (2.F * (yy + zz)), 2.F * (xy - wz), 2.F * (xz + wy) };
  mat.at(1) = { 2.F * (xy + wz), 1.F - (2.F * (xx + zz)), 2.F * (yz - wx) };
  mat.at(2) = { 2.F * (xz - wy), 2.F * (yz + wx), 1.F - (2.F * (xx + yy)) };
  return mat;
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

/// @return Unit quaternion equivalent of rotation matrix.
/// Ref: Shepperd, "Quaternion from rotation matrix", J. Guidance and Control, 1978
inline auto fromMatrix(const Mat3& mat) -> Quat {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  const auto m00 = mat.at(0).at(0);
  const auto m01 = mat.at(0).at(1);
  const auto m02 = mat.at(0).at(2);
  const auto m10 = mat.at(1).at(0);
  const auto m11 = mat.at(1).at(1);
  const auto m12 = mat.at(1).at(2);
  const auto m20 = mat.at(2).at(0);
  const auto m21 = mat.at(2).at(1);
  const auto m22 = mat.at(2).at(2);
  const auto trace = m00 + m11 + m22;
  auto quat = Quat{};
  if (trace > 0.F) {
    const auto den = 2.F * std::sqrt(1.F + trace);
    quat = {
      .w = 0.25F * den,
      .x = (m21 - m12) / den,
      .y = (m02 - m20) / den,
      .z = (m10 - m01) / den,
    };
  } else if (m00 > m11 and m00 > m22) {
    const auto den = 2.F * std::sqrt(1.F + m00 - m11 - m22);
    quat = {
      .w = (m21 - m12) / den,
      .x = 0.25F * den,
      .y = (m01 + m10) / den,
      .z = (m02 + m20) / den,
    };
  } else if (m11 > m22) {
    const auto den = 2.F * std::sqrt(1.F + m11 - m00 - m22);
    quat = {
      .w = (m02 - m20) / den,
      .x = (m01 + m10) / den,
      .y = 0.25F * den,
      .z = (m12 + m21) / den,
    };
  } else {
    const auto den = 2.F * std::sqrt(1.F + m22 - m00 - m11);
    quat = {
      .w = (m10 - m01) / den,
      .x = (m02 + m20) / den,
      .y = (m12 + m21) / den,
      .z = 0.25F * den,
    };
  }
  return normalize(quat);
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

//=================================================================================================
/// 4x4 affine transform, stored column-major so it can be uploaded to shaders directly
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
constexpr auto transformPoint(const Mat4& tf, const Vec3& pt) -> Vec3 {
  return {
    .x = (tf.at(0, 0) * pt.x) + (tf.at(0, 1) * pt.y) + (tf.at(0, 2) * pt.z) + tf.at(0, 3),
    .y = (tf.at(1, 0) * pt.x) + (tf.at(1, 1) * pt.y) + (tf.at(1, 2) * pt.z) + tf.at(1, 3),
    .z = (tf.at(2, 0) * pt.x) + (tf.at(2, 1) * pt.y) + (tf.at(2, 2) * pt.z) + tf.at(2, 3),
  };
}

}  // namespace grape::splat
