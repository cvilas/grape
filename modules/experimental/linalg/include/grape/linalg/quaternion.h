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
    .x = quat.x * inv_norm, .y = quat.y * inv_norm, .z = quat.z * inv_norm, .w = quat.w * inv_norm
  };
}

//-------------------------------------------------------------------------------------------------
/// Multiplicative inverse (Trawny Eq. 21), such that q * inverse(q) == inverse(q) * q == identity
/// (Trawny Eq. 22). Precondition: q must be nonzero.
template <std::floating_point ScalarType>
constexpr auto inverse(const Quaternion<ScalarType>& quat) -> Quaternion<ScalarType> {
  const auto inv_norm_sq = ScalarType{ 1 } / normSquared(quat);
  const auto conj = conjugate(quat);
  return { .x = conj.x * inv_norm_sq,
           .y = conj.y * inv_norm_sq,
           .z = conj.z * inv_norm_sq,
           .w = conj.w * inv_norm_sq };
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

}  // namespace grape::linalg
