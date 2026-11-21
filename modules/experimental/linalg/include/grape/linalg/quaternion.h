//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cmath>
#include <concepts>

#include "grape/linalg/matrix.h"

namespace grape::linalg {

//=================================================================================================
/// Quaternion q = [x, y, z, w] (w scalar), with the algebra of Trawny & Roumeliotis
/// (docs/trawny.pdf; equation numbers below refer to it). Default: identity. Not necessarily unit
/// norm, hence a plain aggregate: Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 }.
template <std::floating_point ScalarType = double>
struct Quaternion {
  ScalarType x{ 0 };
  ScalarType y{ 0 };
  ScalarType z{ 0 };
  ScalarType w{ 1 };

  /// @return Rotation by angle (radians) about unit vector axis (Eq. 86)
  static constexpr auto fromAxisAngle(const Vector<3, ScalarType>& axis, ScalarType angle)
      -> Quaternion;

  /// @return Unit quaternion q with rotationMatrix(q) == mat, up to sign. Precondition: mat is a
  /// rotation matrix.
  static constexpr auto fromMatrix(const Matrix<3, 3, ScalarType>& mat) -> Quaternion;
};

using Quaterniond = Quaternion<double>;
using Quaternionf = Quaternion<float>;

//-------------------------------------------------------------------------------------------------
/// Quaternion product (Eq. 9/10). rhs is applied first: C(lhs * rhs) = C(lhs) * C(rhs) (Eq. 63),
/// matching Rotation composition.
template <std::floating_point ScalarType>
constexpr auto operator*(const Quaternion<ScalarType>& lhs, const Quaternion<ScalarType>& rhs)
    -> Quaternion<ScalarType> {
  using detail::multiplyAdd;
  return {
    .x = multiplyAdd(lhs.w, rhs.x,
                     multiplyAdd(rhs.w, lhs.x, multiplyAdd(-lhs.y, rhs.z, lhs.z * rhs.y))),
    .y = multiplyAdd(lhs.w, rhs.y,
                     multiplyAdd(rhs.w, lhs.y, multiplyAdd(-lhs.z, rhs.x, lhs.x * rhs.z))),
    .z = multiplyAdd(lhs.w, rhs.z,
                     multiplyAdd(rhs.w, lhs.z, multiplyAdd(-lhs.x, rhs.y, lhs.y * rhs.x))),
    .w = multiplyAdd(lhs.w, rhs.w,
                     -multiplyAdd(lhs.x, rhs.x, multiplyAdd(lhs.y, rhs.y, lhs.z * rhs.z))),
  };
}

//-------------------------------------------------------------------------------------------------
/// Conjugate (Eq. 21). The inverse rotation, for a unit quaternion.
template <std::floating_point ScalarType>
constexpr auto conjugate(const Quaternion<ScalarType>& quat) -> Quaternion<ScalarType> {
  return { .x = -quat.x, .y = -quat.y, .z = -quat.z, .w = quat.w };
}

//-------------------------------------------------------------------------------------------------
/// Squared norm (Eq. 5)
template <std::floating_point ScalarType>
constexpr auto normSquared(const Quaternion<ScalarType>& quat) -> ScalarType {
  using detail::multiplyAdd;
  return multiplyAdd(quat.x, quat.x,
                     multiplyAdd(quat.y, quat.y, multiplyAdd(quat.z, quat.z, quat.w * quat.w)));
}

//-------------------------------------------------------------------------------------------------
/// Norm (Eq. 5)
template <std::floating_point ScalarType>
constexpr auto norm(const Quaternion<ScalarType>& quat) -> ScalarType {
  return std::sqrt(normSquared(quat));
}

//-------------------------------------------------------------------------------------------------
/// @return q scaled to unit norm. Precondition: q is nonzero.
template <std::floating_point ScalarType>
constexpr auto normalized(const Quaternion<ScalarType>& quat) -> Quaternion<ScalarType> {
  const auto inv_norm = ScalarType{ 1 } / norm(quat);
  return {
    .x = quat.x * inv_norm,
    .y = quat.y * inv_norm,
    .z = quat.z * inv_norm,
    .w = quat.w * inv_norm,
  };
}

//-------------------------------------------------------------------------------------------------
/// Multiplicative inverse (Eq. 21, 22). Precondition: q is nonzero.
template <std::floating_point ScalarType>
constexpr auto inverse(const Quaternion<ScalarType>& quat) -> Quaternion<ScalarType> {
  const auto inv_norm_sq = ScalarType{ 1 } / normSquared(quat);
  const auto conj = conjugate(quat);
  return {
    .x = conj.x * inv_norm_sq,
    .y = conj.y * inv_norm_sq,
    .z = conj.z * inv_norm_sq,
    .w = conj.w * inv_norm_sq,
  };
}

//-------------------------------------------------------------------------------------------------
/// Rotation matrix C(q) (Eq. 62): C(q) * p expresses p in the frame reached by rotation q.
/// Precondition: q is unit norm; otherwise the result is not a rotation matrix.
template <std::floating_point ScalarType>
constexpr auto rotationMatrix(const Quaternion<ScalarType>& quat) -> Matrix<3, 3, ScalarType> {
  using detail::multiplyAdd;
  constexpr auto TWO = ScalarType{ 2 };
  // ww = 2w^2 - 1 is the unit-norm form of the diagonal term
  const auto ww = multiplyAdd(TWO * quat.w, quat.w, ScalarType{ -1 });
  auto mat = Matrix<3, 3, ScalarType>{};
  mat[0, 0] = multiplyAdd(TWO * quat.x, quat.x, ww);
  mat[1, 1] = multiplyAdd(TWO * quat.y, quat.y, ww);
  mat[2, 2] = multiplyAdd(TWO * quat.z, quat.z, ww);
  mat[0, 1] = TWO * multiplyAdd(quat.w, quat.z, quat.x * quat.y);
  mat[1, 0] = TWO * multiplyAdd(-quat.w, quat.z, quat.x * quat.y);
  mat[0, 2] = TWO * multiplyAdd(-quat.w, quat.y, quat.x * quat.z);
  mat[2, 0] = TWO * multiplyAdd(quat.w, quat.y, quat.x * quat.z);
  mat[1, 2] = TWO * multiplyAdd(quat.w, quat.x, quat.y * quat.z);
  mat[2, 1] = TWO * multiplyAdd(-quat.w, quat.x, quat.y * quat.z);
  return mat;
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point ScalarType>
constexpr auto Quaternion<ScalarType>::fromAxisAngle(const Vector<3, ScalarType>& axis,
                                                     ScalarType angle) -> Quaternion {
  const auto half_angle = angle / ScalarType{ 2 };
  const auto sn = std::sin(half_angle);
  return { .x = axis[0] * sn, .y = axis[1] * sn, .z = axis[2] * sn, .w = std::cos(half_angle) };
}

//-------------------------------------------------------------------------------------------------
/// Shepperd's method (J. Guidance and Control 1(3), 1978). Each branch builds 4 * q_k * q for the
/// largest component q_k, from sums and differences of mat's elements (Eq. 62), then normalises
/// it. This avoids square roots and tolerates slightly non-orthonormal input.
template <std::floating_point ScalarType>
constexpr auto Quaternion<ScalarType>::fromMatrix(const Matrix<3, 3, ScalarType>& mat)
    -> Quaternion {
  constexpr auto ONE = ScalarType{ 1 };
  const auto c00 = mat[0, 0];
  const auto c11 = mat[1, 1];
  const auto c22 = mat[2, 2];
  const auto tr = c00 + c11 + c22;
  auto scaled = Quaternion{};
  if ((tr >= c00) && (tr >= c11) && (tr >= c22)) {  // w largest
    scaled = {
      .x = mat[1, 2] - mat[2, 1],
      .y = mat[2, 0] - mat[0, 2],
      .z = mat[0, 1] - mat[1, 0],
      .w = ONE + tr,
    };
  } else if ((c00 >= c11) && (c00 >= c22)) {  // x largest
    scaled = {
      .x = ONE + c00 - c11 - c22,
      .y = mat[0, 1] + mat[1, 0],
      .z = mat[0, 2] + mat[2, 0],
      .w = mat[1, 2] - mat[2, 1],
    };
  } else if (c11 >= c22) {  // y largest
    scaled = {
      .x = mat[0, 1] + mat[1, 0],
      .y = ONE + c11 - c00 - c22,
      .z = mat[1, 2] + mat[2, 1],
      .w = mat[2, 0] - mat[0, 2],
    };
  } else {  // z largest
    scaled = {
      .x = mat[0, 2] + mat[2, 0],
      .y = mat[1, 2] + mat[2, 1],
      .z = ONE + c22 - c00 - c11,
      .w = mat[0, 1] - mat[1, 0],
    };
  }
  return normalized(scaled);
}

}  // namespace grape::linalg
