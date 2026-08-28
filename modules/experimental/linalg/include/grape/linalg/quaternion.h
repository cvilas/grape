//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cmath>
#include <concepts>

#include "grape/linalg/matrix.h"

namespace grape::linalg {

//=================================================================================================
/// Unit quaternion for representing 3D rotations.
///
/// Implements the quaternion algebra of N. Trawny, S. I. Roumeliotis, "Indirect Kalman Filter for
/// 3D Attitude Estimation", University of Minnesota, Dept. of CSE, TR-2005-002, 2005 (see
/// ahrs/docs/trawny.pdf, Section 1). Layout is q = [x, y, z, w]^T = [q_vec; q4] (Eq. 1), where
/// q_vec is the vector part and w is the scalar part. A default-constructed Quaternion is the
/// multiplicative identity q0 = [0, 0, 0, 1]^T (Eq. 19).
///
/// @tparam ScalarType Floating point type used for quaternion elements
template <std::floating_point ScalarType = double>
struct Quaternion {
  ScalarType x{ 0 };
  ScalarType y{ 0 };
  ScalarType z{ 0 };
  ScalarType w{ 1 };
};

using Quaterniond = Quaternion<double>;

//-------------------------------------------------------------------------------------------------
/// Quaternion multiplication lhs (x) rhs (Trawny Eq. 9/10). Composes rotations: if lhs rotates
/// frame A to B and rhs rotates frame B to C, then (lhs * rhs) rotates frame A to C.
template <std::floating_point ScalarType>
constexpr auto operator*(const Quaternion<ScalarType>& lhs, const Quaternion<ScalarType>& rhs)
    -> Quaternion<ScalarType> {
  return {
    .x = std::fma(lhs.w, rhs.x, std::fma(rhs.w, lhs.x, std::fma(-lhs.y, rhs.z, lhs.z * rhs.y))),
    .y = std::fma(lhs.w, rhs.y, std::fma(rhs.w, lhs.y, std::fma(-lhs.z, rhs.x, lhs.x * rhs.z))),
    .z = std::fma(lhs.w, rhs.z, std::fma(rhs.w, lhs.z, std::fma(-lhs.x, rhs.y, lhs.y * rhs.x))),
    .w = std::fma(lhs.w, rhs.w, -std::fma(lhs.x, rhs.x, std::fma(lhs.y, rhs.y, lhs.z * rhs.z))),
  };
}

//-------------------------------------------------------------------------------------------------
/// Conjugate quaternion [-q_vec; q4] (Trawny Eq. 21). For a unit quaternion this equals its
/// inverse and represents the inverse rotation.
template <std::floating_point ScalarType>
constexpr auto conjugate(const Quaternion<ScalarType>& quat) -> Quaternion<ScalarType> {
  return { .x = -quat.x, .y = -quat.y, .z = -quat.z, .w = quat.w };
}

//-------------------------------------------------------------------------------------------------
/// Squared norm |q|^2 = q_vec.q_vec + q4^2 (Trawny Eq. 5)
template <std::floating_point ScalarType>
constexpr auto normSquared(const Quaternion<ScalarType>& quat) -> ScalarType {
  return std::fma(quat.x, quat.x,
                  std::fma(quat.y, quat.y, std::fma(quat.z, quat.z, quat.w * quat.w)));
}

//-------------------------------------------------------------------------------------------------
/// Norm |q| (Trawny Eq. 5). A rotation quaternion satisfies |q| == 1.
template <std::floating_point ScalarType>
constexpr auto norm(const Quaternion<ScalarType>& quat) -> ScalarType {
  return std::sqrt(normSquared(quat));
}

//-------------------------------------------------------------------------------------------------
/// Rescale to unit norm. Precondition: q must be nonzero.
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
/// Multiplicative inverse (Trawny Eq. 21), such that q * inverse(q) == inverse(q) * q == identity
/// (Trawny Eq. 22). Precondition: q must be nonzero.
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
/// Equivalent rotation matrix C(q) (Trawny Eq. 62), such that for a vector p, C(q) * p expresses
/// p in the frame reached by rotation q.
template <std::floating_point ScalarType>
constexpr auto rotationMatrix(const Quaternion<ScalarType>& quat) -> Matrix<3, 3, ScalarType> {
  constexpr auto TWO = ScalarType{ 2 };
  // ww == 2*w^2 - 1; off-diagonal terms fuse the cross product with the scalar-vector product
  // before the final scale-by-two, halving the multiply count versus computing each term whole.
  const auto ww = std::fma(TWO * quat.w, quat.w, ScalarType{ -1 });
  auto mat = Matrix<3, 3, ScalarType>{};
  mat[0, 0] = std::fma(TWO * quat.x, quat.x, ww);
  mat[1, 1] = std::fma(TWO * quat.y, quat.y, ww);
  mat[2, 2] = std::fma(TWO * quat.z, quat.z, ww);
  mat[0, 1] = TWO * std::fma(quat.w, quat.z, quat.x * quat.y);
  mat[1, 0] = TWO * std::fma(-quat.w, quat.z, quat.x * quat.y);
  mat[0, 2] = TWO * std::fma(-quat.w, quat.y, quat.x * quat.z);
  mat[2, 0] = TWO * std::fma(quat.w, quat.y, quat.x * quat.z);
  mat[1, 2] = TWO * std::fma(quat.w, quat.x, quat.y * quat.z);
  mat[2, 1] = TWO * std::fma(-quat.w, quat.x, quat.y * quat.z);
  return mat;
}

//-------------------------------------------------------------------------------------------------
/// Quaternion of a rotation by an angle about an axis (Trawny Eq. 86): rotating a frame by angle
/// (radians) about the unit vector axis gives q = [axis * sin(angle/2); cos(angle/2)].
/// Precondition: axis is a unit vector.
template <std::floating_point ScalarType>
constexpr auto fromAxisAngle(const Vector<3, ScalarType>& axis, ScalarType angle)
    -> Quaternion<ScalarType> {
  const auto half_angle = angle / ScalarType{ 2 };
  const auto sn = std::sin(half_angle);
  return { .x = axis[0] * sn, .y = axis[1] * sn, .z = axis[2] * sn, .w = std::cos(half_angle) };
}

//-------------------------------------------------------------------------------------------------
/// Unit quaternion equivalent of rotation matrix C, inverting rotationMatrix() up to sign (q and
/// -q represent the same rotation).
///
/// Uses Shepperd's method: each quaternion component can be recovered from the diagonal, and the
/// rest follow by dividing sums/differences of off-diagonal pairs by it. Of the four algebraically
/// equivalent routes, take the one that divides by the largest component, so the division never
/// amplifies rounding error. The result is normalised, so inputs that are slightly off (e.g.
/// decoded from quantised storage) still yield a unit quaternion.
/// Ref: S. W. Shepperd, "Quaternion from rotation matrix", J. Guidance and Control 1(3), 1978
/// Precondition: mat is a rotation matrix (orthonormal, determinant +1)
template <std::floating_point ScalarType>
constexpr auto toQuaternion(const Matrix<3, 3, ScalarType>& mat) -> Quaternion<ScalarType> {
  constexpr auto ONE = ScalarType{ 1 };
  constexpr auto TWO = ScalarType{ 2 };
  constexpr auto QUARTER = ScalarType{ 0.25 };
  const auto tr = trace(mat);
  auto quat = Quaternion<ScalarType>{};
  if (tr > ScalarType{ 0 }) {
    const auto den = TWO * std::sqrt(ONE + tr);
    quat = {
      .x = (mat[1, 2] - mat[2, 1]) / den,
      .y = (mat[2, 0] - mat[0, 2]) / den,
      .z = (mat[0, 1] - mat[1, 0]) / den,
      .w = QUARTER * den,
    };
  } else if ((mat[0, 0] > mat[1, 1]) && (mat[0, 0] > mat[2, 2])) {
    const auto den = TWO * std::sqrt(ONE + mat[0, 0] - mat[1, 1] - mat[2, 2]);
    quat = {
      .x = QUARTER * den,
      .y = (mat[0, 1] + mat[1, 0]) / den,
      .z = (mat[0, 2] + mat[2, 0]) / den,
      .w = (mat[1, 2] - mat[2, 1]) / den,
    };
  } else if (mat[1, 1] > mat[2, 2]) {
    const auto den = TWO * std::sqrt(ONE + mat[1, 1] - mat[0, 0] - mat[2, 2]);
    quat = {
      .x = (mat[0, 1] + mat[1, 0]) / den,
      .y = QUARTER * den,
      .z = (mat[1, 2] + mat[2, 1]) / den,
      .w = (mat[2, 0] - mat[0, 2]) / den,
    };
  } else {
    const auto den = TWO * std::sqrt(ONE + mat[2, 2] - mat[0, 0] - mat[1, 1]);
    quat = {
      .x = (mat[0, 2] + mat[2, 0]) / den,
      .y = (mat[1, 2] + mat[2, 1]) / den,
      .z = QUARTER * den,
      .w = (mat[0, 1] - mat[1, 0]) / den,
    };
  }
  return normalized(quat);
}

}  // namespace grape::linalg
