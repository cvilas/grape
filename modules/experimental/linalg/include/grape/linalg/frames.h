//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <limits>
#include <optional>

#include "grape/linalg/matrix.h"
#include "grape/linalg/quaternion.h"
#include "grape/utils/attributes.h"

namespace grape::linalg {

//=================================================================================================
// Compile-time coordinate-frame-aware geometry for rigid body motion.
//
// Frame and point semantics are encoded in types, so mixing incompatible frames fails to compile.
// Tags occupy no storage. Conventions follow P. Furgale, "Representing Robot Pose: The Good, the
// Bad, and the Ugly" (https://rpg.ifi.uzh.ch/docs/teaching/2025/FurgaleTutorial.pdf):
//
//   Math       | C++                     | Meaning
//   -----------|-------------------------|------------------------------------------------------
//   _E v       | Vector3<E>              | Free vector expressed in E
//   _E r_AB    | Displacement<E, A, B>   | Displacement from point A to point B, expressed in E
//   _E r_EP    | Position<E, P>          | Position of P relative to E's origin, expressed in E
//   C_AB       | Rotation<A, B>          | Re-expresses B coordinates in A
//   T_AB       | Transform<A, B>         | Maps B-relative positions to A-relative positions
//
// Rotations and transforms use destination-source template parameter order. Displacements use
// basis, tail, head order. Rotating a displacement changes its basis, not its endpoints; these
// are passive coordinate changes, not active motion of a physical point.
//
// The compiler proves compatibility of declared semantics, not the physical truth of labels or
// numeric accuracy. Raw coordinates enter through explicit constructors; it cannot know that
// coordinates supplied as Vector3<World> actually came from another frame.
//=================================================================================================

//-------------------------------------------------------------------------------------------------
/// Base tag for named physical points (landmarks, sensor origins, etc.). Derive a distinct tag
/// for each distinct physical point.
struct PointTag {};

/// Base tag for coordinate frames. A frame is also a point, denoting its origin. Deriving one
/// user-defined frame from another does not make their coordinates compatible.
struct Frame : PointTag {};

/// Concept for named physical points
template <typename T>
concept PointType = std::derived_from<T, PointTag>;

/// Concept for coordinate frames
template <typename T>
concept FrameType = std::derived_from<T, Frame>;

//-------------------------------------------------------------------------------------------------
/// @return Direction cosine matrix for a passive rotation about the X axis: for a frame B reached
/// by rotating frame A by angle (radians) about its x-axis, v_B = rx(angle) * v_A re-expresses a
/// vector from A to B. Consistent with rotationMatrix() of the equivalent quaternion.
template <std::floating_point Scalar>
constexpr auto rx(Scalar angle) -> Matrix<3, 3, Scalar> {
  const auto cs = std::cos(angle);
  const auto sn = std::sin(angle);
  const auto zero = Scalar{ 0 };
  const auto one = Scalar{ 1 };
  return Matrix<3, 3, Scalar>{
    one,  zero, zero,  //
    zero, cs,   sn,    //
    zero, -sn,  cs,
  };
}

//-------------------------------------------------------------------------------------------------
/// @return Direction cosine matrix for a passive rotation about the Y axis (see rx())
template <std::floating_point Scalar>
constexpr auto ry(Scalar angle) -> Matrix<3, 3, Scalar> {
  const auto cs = std::cos(angle);
  const auto sn = std::sin(angle);
  const auto zero = Scalar{ 0 };
  const auto one = Scalar{ 1 };
  return Matrix<3, 3, Scalar>{
    cs,   zero, -sn,   //
    zero, one,  zero,  //
    sn,   zero, cs,
  };
}

//-------------------------------------------------------------------------------------------------
/// @return Direction cosine matrix for a passive rotation about the Z axis (see rx())
template <std::floating_point Scalar>
constexpr auto rz(Scalar angle) -> Matrix<3, 3, Scalar> {
  const auto cs = std::cos(angle);
  const auto sn = std::sin(angle);
  const auto zero = Scalar{ 0 };
  const auto one = Scalar{ 1 };
  return Matrix<3, 3, Scalar>{
    cs,   sn,   zero,  //
    -sn,  cs,   zero,  //
    zero, zero, one,
  };
}

//=================================================================================================
/// Free vector (direction, velocity, angular rate, etc.) expressed in frame ExpressedIn. Has
/// magnitude and direction but no anchor point.
template <FrameType ExpressedIn, std::floating_point Scalar = double>
class Vector3 {
public:
  constexpr Vector3() = default;
  constexpr explicit Vector3(const Vector<3, Scalar>& coordinates) : coordinates_(coordinates) {
  }
  constexpr Vector3(Scalar cx, Scalar cy, Scalar cz) : coordinates_{ cx, cy, cz } {
  }

  /// @return Raw coordinates. Reconstructing a differently-tagged object from these is a
  /// deliberate escape hatch.
  [[nodiscard]] constexpr auto coordinates() const GRAPE_LIFETIMEBOUND -> const Vector<3, Scalar>& {
    return coordinates_;
  }

private:
  Vector<3, Scalar> coordinates_{};
};

//-------------------------------------------------------------------------------------------------
/// @return Sum of free vectors sharing a basis
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator+(const Vector3<ExpressedIn, Scalar>& lhs,
                         const Vector3<ExpressedIn, Scalar>& rhs) -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ lhs.coordinates() + rhs.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Negation of a free vector
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator-(const Vector3<ExpressedIn, Scalar>& vec) -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ -vec.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Difference of free vectors sharing a basis
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator-(const Vector3<ExpressedIn, Scalar>& lhs,
                         const Vector3<ExpressedIn, Scalar>& rhs) -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ lhs.coordinates() - rhs.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Free vector scaled by factor
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator*(const Vector3<ExpressedIn, Scalar>& vec, Scalar factor)
    -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ vec.coordinates() * factor };
}

//-------------------------------------------------------------------------------------------------
/// @return Free vector scaled by factor
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator*(Scalar factor, const Vector3<ExpressedIn, Scalar>& vec)
    -> Vector3<ExpressedIn, Scalar> {
  return vec * factor;
}

//-------------------------------------------------------------------------------------------------
/// @return Dot product of free vectors sharing a basis
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto dot(const Vector3<ExpressedIn, Scalar>& lhs, const Vector3<ExpressedIn, Scalar>& rhs)
    -> Scalar {
  return dot(lhs.coordinates(), rhs.coordinates());
}

//-------------------------------------------------------------------------------------------------
/// @return Cross product of free vectors sharing a basis
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto cross(const Vector3<ExpressedIn, Scalar>& lhs,
                     const Vector3<ExpressedIn, Scalar>& rhs) -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ cross(lhs.coordinates(), rhs.coordinates()) };
}

//=================================================================================================
/// _E r_AB: Displacement from point Tail to point Head, expressed in frame ExpressedIn. Supports
/// ordered head-to-tail chaining rather than every commuted form of addition.
template <FrameType ExpressedIn, PointType Tail, PointType Head,
          std::floating_point Scalar = double>
class Displacement {
public:
  constexpr Displacement() = default;
  constexpr explicit Displacement(const Vector<3, Scalar>& coordinates)
    : coordinates_(coordinates) {
  }
  constexpr Displacement(Scalar cx, Scalar cy, Scalar cz) : coordinates_{ cx, cy, cz } {
  }

  /// @return Raw coordinates
  [[nodiscard]] constexpr auto coordinates() const GRAPE_LIFETIMEBOUND -> const Vector<3, Scalar>& {
    return coordinates_;
  }

private:
  Vector<3, Scalar> coordinates_{};
};

//-------------------------------------------------------------------------------------------------
/// @return Head-to-tail chained displacement: r_AB + r_BC = r_AC
template <FrameType ExpressedIn, PointType Tail, PointType Mid, PointType Head,
          std::floating_point Scalar>
constexpr auto operator+(const Displacement<ExpressedIn, Tail, Mid, Scalar>& lhs,
                         const Displacement<ExpressedIn, Mid, Head, Scalar>& rhs)
    -> Displacement<ExpressedIn, Tail, Head, Scalar> {
  return Displacement<ExpressedIn, Tail, Head, Scalar>{ lhs.coordinates() + rhs.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Reversed displacement: -r_AB = r_BA
template <FrameType ExpressedIn, PointType Tail, PointType Head, std::floating_point Scalar>
constexpr auto operator-(const Displacement<ExpressedIn, Tail, Head, Scalar>& disp)
    -> Displacement<ExpressedIn, Head, Tail, Scalar> {
  return Displacement<ExpressedIn, Head, Tail, Scalar>{ -disp.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Difference of displacements sharing a head: r_AC - r_BC = r_AB
template <FrameType ExpressedIn, PointType Tail, PointType OtherTail, PointType Head,
          std::floating_point Scalar>
constexpr auto operator-(const Displacement<ExpressedIn, Tail, Head, Scalar>& lhs,
                         const Displacement<ExpressedIn, OtherTail, Head, Scalar>& rhs)
    -> Displacement<ExpressedIn, Tail, OtherTail, Scalar> {
  return lhs + (-rhs);
}

//=================================================================================================
/// _E r_EP: Position of point P relative to the origin of frame ExpressedIn, expressed in
/// ExpressedIn. Positions cannot be added or implicitly treated as vectors.
template <FrameType ExpressedIn, PointType Point, std::floating_point Scalar = double>
class Position {
public:
  constexpr Position() = default;
  constexpr explicit Position(const Vector<3, Scalar>& coordinates) : coordinates_(coordinates) {
  }
  constexpr Position(Scalar cx, Scalar cy, Scalar cz) : coordinates_{ cx, cy, cz } {
  }

  /// @return Raw coordinates
  [[nodiscard]] constexpr auto coordinates() const GRAPE_LIFETIMEBOUND -> const Vector<3, Scalar>& {
    return coordinates_;
  }

private:
  Vector<3, Scalar> coordinates_{};
};

//-------------------------------------------------------------------------------------------------
/// @return Displacement between points: r_EP - r_EQ = r_QP
template <FrameType ExpressedIn, PointType Point, PointType Other, std::floating_point Scalar>
constexpr auto operator-(const Position<ExpressedIn, Point, Scalar>& lhs,
                         const Position<ExpressedIn, Other, Scalar>& rhs)
    -> Displacement<ExpressedIn, Other, Point, Scalar> {
  return Displacement<ExpressedIn, Other, Point, Scalar>{ lhs.coordinates() - rhs.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Position displaced from P to Other: r_EP + r_P,Other = r_E,Other
template <FrameType ExpressedIn, PointType Point, PointType Other, std::floating_point Scalar>
constexpr auto operator+(const Position<ExpressedIn, Point, Scalar>& pos,
                         const Displacement<ExpressedIn, Point, Other, Scalar>& disp)
    -> Position<ExpressedIn, Other, Scalar> {
  return Position<ExpressedIn, Other, Scalar>{ pos.coordinates() + disp.coordinates() };
}

//-------------------------------------------------------------------------------------------------
/// @return Position displaced against a displacement that ends at P: r_EP - r_Other,P = r_E,Other
template <FrameType ExpressedIn, PointType Point, PointType Other, std::floating_point Scalar>
constexpr auto operator-(const Position<ExpressedIn, Point, Scalar>& pos,
                         const Displacement<ExpressedIn, Other, Point, Scalar>& disp)
    -> Position<ExpressedIn, Other, Scalar> {
  return pos + (-disp);
}

//=================================================================================================
/// C_AB: Rotation that re-expresses From-frame (B) coordinates in To-frame (A) coordinates:
/// v_To = C * v_From. A default-constructed Rotation represents aligned axes.
///
/// Construction is validated or derived from trusted sources; composition and inversion preserve
/// the rotation structure without revalidation (long chains still accumulate floating point
/// drift).
template <FrameType To, FrameType From, std::floating_point Scalar = double>
class Rotation {
public:
  /// Default tolerance for orthonormality validation in fromMatrix()
  static constexpr auto DEFAULT_TOLERANCE =
      Scalar{ 10000 } * std::numeric_limits<Scalar>::epsilon();

  /// Represents aligned axes, including between distinct frame tags. This default is an explicit
  /// geometric assumption, not a fact inferred about the system.
  constexpr Rotation() = default;

  /// Creates a rotation from a direction cosine matrix after validating it is orthonormal with
  /// determinant +1 (within tolerance) and contains only finite values.
  /// @return Validated rotation, or std::nullopt if mat is not a rotation matrix
  static constexpr auto fromMatrix(const Matrix<3, 3, Scalar>& mat,
                                   Scalar tolerance = DEFAULT_TOLERANCE)
      -> std::optional<Rotation> {
    // Checks are written such that NaN comparisons (always false) reject the matrix
    const auto gram = mat * transpose(mat);
    const auto ident = identity<3, Scalar>();
    for (std::size_t row = 0UZ; row < 3UZ; ++row) {
      for (std::size_t col = 0UZ; col < 3UZ; ++col) {
        if (!(detail::absolute(gram[row, col] - ident[row, col]) <= tolerance)) {
          return std::nullopt;
        }
      }
    }
    if (!(detail::absolute(determinant(mat) - Scalar{ 1 }) <= tolerance)) {
      return std::nullopt;
    }
    return Rotation{ mat };
  }

  /// Creates the rotation from the quaternion that rotates frame From into frame To (Trawny
  /// convention): the result is C(q) of rotationMatrix(), mapping From coordinates to To
  /// coordinates. Precondition: q is a unit quaternion.
  static constexpr auto fromQuaternion(const Quaternion<Scalar>& quat) -> Rotation {
    return Rotation{ rotationMatrix(quat) };
  }

  /// @return Rotation for frame To reached by rotating frame From by angle (radians) about its
  /// x-axis
  static constexpr auto aboutX(Scalar angle) -> Rotation {
    return Rotation{ rx(angle) };
  }

  /// @return Rotation for frame To reached by rotating frame From by angle (radians) about its
  /// y-axis
  static constexpr auto aboutY(Scalar angle) -> Rotation {
    return Rotation{ ry(angle) };
  }

  /// @return Rotation for frame To reached by rotating frame From by angle (radians) about its
  /// z-axis
  static constexpr auto aboutZ(Scalar angle) -> Rotation {
    return Rotation{ rz(angle) };
  }

  /// @return Underlying direction cosine matrix (row-major, acts on column vectors)
  [[nodiscard]] constexpr auto matrix() const GRAPE_LIFETIMEBOUND -> const Matrix<3, 3, Scalar>& {
    return matrix_;
  }

  /// @return Composed rotation C_AC = C_AB * C_BC
  template <FrameType Source>
  constexpr auto operator*(const Rotation<From, Source, Scalar>& other) const
      -> Rotation<To, Source, Scalar> {
    return Rotation<To, Source, Scalar>{ matrix_ * other.matrix() };
  }

  /// @return Free vector re-expressed in To coordinates
  constexpr auto operator*(const Vector3<From, Scalar>& vec) const -> Vector3<To, Scalar> {
    return Vector3<To, Scalar>{ matrix_ * vec.coordinates() };
  }

  /// @return Displacement re-expressed in To coordinates. The basis changes; endpoints do not.
  template <PointType Tail, PointType Head>
  constexpr auto operator*(const Displacement<From, Tail, Head, Scalar>& disp) const
      -> Displacement<To, Tail, Head, Scalar> {
    return Displacement<To, Tail, Head, Scalar>{ matrix_ * disp.coordinates() };
  }

  /// @return Inverse rotation C_BA = transpose(C_AB)
  [[nodiscard]] constexpr auto inverse() const -> Rotation<From, To, Scalar> {
    return Rotation<From, To, Scalar>{ transpose(matrix_) };
  }

private:
  template <FrameType, FrameType, std::floating_point>
  friend class Rotation;

  constexpr explicit Rotation(const Matrix<3, 3, Scalar>& mat) : matrix_(mat) {
  }

  Matrix<3, 3, Scalar> matrix_{ identity<3, Scalar>() };
};

//=================================================================================================
/// T_AB = [C_AB, _A r_AB; 0, 1]: Rigid body transform that maps From-relative (B) positions to
/// To-relative (A) positions: _A r_AP = C_AB * _B r_BP + _A r_AB. A default-constructed Transform
/// assumes aligned axes and coincident origins.
template <FrameType To, FrameType From, std::floating_point Scalar = double>
class Transform {
public:
  constexpr Transform() = default;

  /// Creates a transform from rotation C_AB and translation _A r_AB (displacement from To's
  /// origin to From's origin, expressed in To)
  constexpr Transform(const Rotation<To, From, Scalar>& rotation,
                      const Displacement<To, To, From, Scalar>& translation)
    : rotation_(rotation), translation_(translation) {
  }

  /// @return Rotation part C_AB
  [[nodiscard]] constexpr auto rotation() const GRAPE_LIFETIMEBOUND
      -> const Rotation<To, From, Scalar>& {
    return rotation_;
  }

  /// @return Translation part _A r_AB
  [[nodiscard]] constexpr auto translation() const GRAPE_LIFETIMEBOUND
      -> const Displacement<To, To, From, Scalar>& {
    return translation_;
  }

  /// @return Composed transform T_AC = T_AB * T_BC
  template <FrameType Source>
  constexpr auto operator*(const Transform<From, Source, Scalar>& other) const
      -> Transform<To, Source, Scalar> {
    return Transform<To, Source, Scalar>{
      rotation_ * other.rotation(),
      translation_ + (rotation_ * other.translation()),
    };
  }

  /// @return Position mapped from From-relative to To-relative: _A r_AP = C_AB _B r_BP + _A r_AB
  template <PointType Point>
  constexpr auto operator*(const Position<From, Point, Scalar>& point) const
      -> Position<To, Point, Scalar> {
    return Position<To, Point, Scalar>{
      (rotation_.matrix() * point.coordinates()) + translation_.coordinates(),
    };
  }

  /// @return Free vector re-expressed in To coordinates (no translation applied)
  constexpr auto operator*(const Vector3<From, Scalar>& vec) const -> Vector3<To, Scalar> {
    return rotation_ * vec;
  }

  /// @return Displacement re-expressed in To coordinates (no translation applied)
  template <PointType Tail, PointType Head>
  constexpr auto operator*(const Displacement<From, Tail, Head, Scalar>& disp) const
      -> Displacement<To, Tail, Head, Scalar> {
    return rotation_ * disp;
  }

  /// @return Inverse transform T_BA with rotation C_AB^T and translation -C_AB^T * _A r_AB
  [[nodiscard]] constexpr auto inverse() const -> Transform<From, To, Scalar> {
    const auto inv_rotation = rotation_.inverse();
    return Transform<From, To, Scalar>{ inv_rotation, inv_rotation * (-translation_) };
  }

private:
  Rotation<To, From, Scalar> rotation_{};
  Displacement<To, To, From, Scalar> translation_{};
};

}  // namespace grape::linalg
