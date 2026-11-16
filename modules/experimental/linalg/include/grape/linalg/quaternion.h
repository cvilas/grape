//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cmath>
#include <concepts>

#include "grape/linalg/matrix.h"

namespace grape::linalg {

//=================================================================================================
/// Quaternion, used chiefly to represent 3D rotations (as unit quaternions).
///
/// Implements the quaternion algebra of N. Trawny, S. I. Roumeliotis, "Indirect Kalman Filter for
/// 3D Attitude Estimation", University of Minnesota, Dept. of CSE, TR-2005-002, 2005 (see
/// docs). Layout is q = [x, y, z, w]^T = [q_vec; q4] (Eq. 1), where q_vec is the vector part and
/// w is the scalar part. A default-constructed Quaternion is the multiplicative identity
/// q0 = [0, 0, 0, 1]^T (Eq. 19).
///
/// A plain aggregate: the components are independent (general quaternions need not have unit
/// norm), so there is no invariant to protect, and designated initialisers make the component
/// order explicit: Quaterniond{ .x = 0, .y = 0, .z = 1, .w = 0 }. Named factories construct
/// rotations; the algebra is provided as free functions below.
///
/// @tparam ScalarType Floating point type used for quaternion elements
template <std::floating_point ScalarType = double>
struct Quaternion {
  ScalarType x{ 0 };
  ScalarType y{ 0 };
  ScalarType z{ 0 };
  ScalarType w{ 1 };

  /// @return Quaternion of a rotation by angle (radians) about the unit vector axis (Trawny
  /// Eq. 86). Precondition: axis is a unit vector.
  static constexpr auto fromAxisAngle(const Vector<3, ScalarType>& axis, ScalarType angle)
      -> Quaternion;

  /// @return Unit quaternion equivalent of rotation matrix mat, inverting rotationMatrix() up to
  /// sign. Precondition: mat is a rotation matrix (orthonormal, determinant +1).
  static constexpr auto fromMatrix(const Matrix<3, 3, ScalarType>& mat) -> Quaternion;
};

using Quaterniond = Quaternion<double>;
using Quaternionf = Quaternion<float>;

//-------------------------------------------------------------------------------------------------
/// Quaternion multiplication lhs (x) rhs (Trawny Eq. 9/10), satisfying
/// C(lhs (x) rhs) = C(lhs) * C(rhs) (Trawny Eq. 63). The right-hand operand is applied first: if
/// rhs rotates frame A to B and lhs rotates frame B to C, then (lhs * rhs) rotates frame A to C.
/// This matches Rotation composition, C_CA = C_CB * C_BA.
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
  using detail::multiplyAdd;
  return multiplyAdd(quat.x, quat.x,
                     multiplyAdd(quat.y, quat.y, multiplyAdd(quat.z, quat.z, quat.w * quat.w)));
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
/// Precondition: q is a unit quaternion. The diagonal uses the 2*w^2 - 1 form of Eq. 62, which
/// holds only when |q| == 1; for any other q the result is not a rotation matrix.
template <std::floating_point ScalarType>
constexpr auto rotationMatrix(const Quaternion<ScalarType>& quat) -> Matrix<3, 3, ScalarType> {
  using detail::multiplyAdd;
  constexpr auto TWO = ScalarType{ 2 };
  // ww == 2*w^2 - 1; off-diagonal terms fuse the cross product with the scalar-vector product
  // before the final scale-by-two, halving the multiply count versus computing each term whole.
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
/// Quaternion of a rotation by an angle about an axis (Trawny Eq. 86): rotating a frame by angle
/// (radians) about the unit vector axis gives q = [axis * sin(angle/2); cos(angle/2)].
/// Precondition: axis is a unit vector.
template <std::floating_point ScalarType>
constexpr auto Quaternion<ScalarType>::fromAxisAngle(const Vector<3, ScalarType>& axis,
                                                     ScalarType angle) -> Quaternion {
  const auto half_angle = angle / ScalarType{ 2 };
  const auto sn = std::sin(half_angle);
  return { .x = axis[0] * sn, .y = axis[1] * sn, .z = axis[2] * sn, .w = std::cos(half_angle) };
}

//-------------------------------------------------------------------------------------------------
/// Unit quaternion equivalent of rotation matrix C, inverting rotationMatrix() up to sign (q and
/// -q represent the same rotation).
///
/// Uses Shepperd's method. From Trawny Eq. 62, the diagonal gives each squared component
/// (4w^2 = 1 + tr, 4x^2 = 1 + 2*C00 - tr, ...) and the off-diagonal sums/differences give the
/// pairwise products (4wx = C12 - C21, 4xy = C01 + C10, ...). Pick the largest component k - the
/// largest of tr, C00, C11, C22 - then the row [4*q_k*x, 4*q_k*y, 4*q_k*z, 4*q_k*w] contains only
/// those known terms and is proportional to q with a scale of at least 1. Normalising it yields q
/// without per-component square roots or divisions, and inputs that are slightly off (e.g. decoded
/// from quantised storage) still yield a unit quaternion.
/// Ref: S. W. Shepperd, "Quaternion from rotation matrix", J. Guidance and Control 1(3), 1978
/// Precondition: mat is a rotation matrix (orthonormal, determinant +1)
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
