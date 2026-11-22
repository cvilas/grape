//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <limits>
#include <optional>
#include <type_traits>

#include "grape/linalg/matrix.h"
#include "grape/linalg/quaternion.h"
#include "grape/utils/attributes.h"

namespace grape::linalg {

//=================================================================================================
// Frame-aware geometry: mixing incompatible frames or points fails to compile. Conventions are in
// docs/design.md.
//
// The types check only the labels you declare. Raw coordinates enter through explicit
// constructors, so the compiler cannot tell that coordinates labelled World came from another
// frame.
//=================================================================================================

//-------------------------------------------------------------------------------------------------
/// Base tag for named points (landmarks, sensor origins). Derive one tag per physical point.
struct PointTag {};

/// Base tag for coordinate frames. A frame is also a point: its origin. Frames derived from one
/// another are still distinct.
struct Frame : PointTag {};

template <typename T>
concept PointType = std::derived_from<T, PointTag>;

template <typename T>
concept FrameType = std::derived_from<T, Frame>;

//-------------------------------------------------------------------------------------------------
/// @return Passive rotation about x: if frame B is frame A rotated by angle (radians) about x,
/// then v_B = rx(angle) * v_A
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

namespace detail {

//-------------------------------------------------------------------------------------------------
/// Storage shared by the tagged types. Inherited constructors exclude the base's copy constructor,
/// so one tagged type cannot be constructed from another.
template <std::floating_point Scalar>
class TaggedCoordinates {
public:
  constexpr TaggedCoordinates() = default;
  constexpr explicit TaggedCoordinates(const Vector<3, Scalar>& coordinates)
    : coordinates_(coordinates) {
  }
  constexpr explicit TaggedCoordinates(Scalar cx, Scalar cy, Scalar cz)
    : coordinates_{ cx, cy, cz } {
  }

  /// @return Raw coordinates. Passing them to another tagged type is a deliberate escape hatch.
  [[nodiscard]] constexpr auto coordinates() const GRAPE_LIFETIMEBOUND -> const Vector<3, Scalar>& {
    return coordinates_;
  }

private:
  Vector<3, Scalar> coordinates_{};
};

}  // namespace detail

//=================================================================================================
/// Free vector (direction, velocity, angular rate) expressed in ExpressedIn. Has no anchor point.
template <FrameType ExpressedIn, std::floating_point Scalar = double>
class Vector3 : public detail::TaggedCoordinates<Scalar> {
public:
  using detail::TaggedCoordinates<Scalar>::TaggedCoordinates;
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
constexpr auto operator*(const Vector3<ExpressedIn, Scalar>& vec,
                         std::type_identity_t<Scalar> factor) -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ vec.coordinates() * factor };
}

//-------------------------------------------------------------------------------------------------
/// @return Free vector scaled by factor
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator*(std::type_identity_t<Scalar> factor,
                         const Vector3<ExpressedIn, Scalar>& vec) -> Vector3<ExpressedIn, Scalar> {
  return vec * factor;
}

//-------------------------------------------------------------------------------------------------
/// @return Free vector divided by divisor
template <FrameType ExpressedIn, std::floating_point Scalar>
constexpr auto operator/(const Vector3<ExpressedIn, Scalar>& vec,
                         std::type_identity_t<Scalar> divisor) -> Vector3<ExpressedIn, Scalar> {
  return Vector3<ExpressedIn, Scalar>{ vec.coordinates() / divisor };
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
/// _E r_AB: displacement from Tail to Head, expressed in ExpressedIn. Adds only head to tail.
template <FrameType ExpressedIn, PointType Tail, PointType Head,
          std::floating_point Scalar = double>
class Displacement : public detail::TaggedCoordinates<Scalar> {
public:
  using detail::TaggedCoordinates<Scalar>::TaggedCoordinates;
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
/// _E r_EP: position of Point relative to the origin of ExpressedIn. Positions cannot be added.
template <FrameType ExpressedIn, PointType Point, std::floating_point Scalar = double>
class Position : public detail::TaggedCoordinates<Scalar> {
public:
  using detail::TaggedCoordinates<Scalar>::TaggedCoordinates;
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
/// C_AB: re-expresses From (B) coordinates in To (A): v_A = C_AB * v_B. Construction is validated;
/// composition and inversion are not revalidated, so long chains accumulate drift.
template <FrameType To, FrameType From, std::floating_point Scalar = double>
class Rotation {
public:
  /// Default tolerance for orthonormality validation in fromMatrix()
  static constexpr auto DEFAULT_TOLERANCE =
      Scalar{ 10000 } * std::numeric_limits<Scalar>::epsilon();

  /// Aligned axes (identity)
  constexpr Rotation() = default;

  /// @return Rotation, or std::nullopt unless mat is finite and orthonormal (within tolerance)
  /// with determinant +1
  static constexpr auto fromMatrix(const Matrix<3, 3, Scalar>& mat,
                                   Scalar tolerance = DEFAULT_TOLERANCE)
      -> std::optional<Rotation> {
    // Comparisons are written so that NaN is rejected
    const auto gram = mat * transpose(mat);
    const auto ident = identity<3, Scalar>();
    for (std::size_t row = 0UZ; row < 3UZ; ++row) {
      for (std::size_t col = 0UZ; col < 3UZ; ++col) {
        if (!(detail::absolute(gram(row, col) - ident(row, col)) <= tolerance)) {
          return std::nullopt;
        }
      }
    }
    // orthonormal: determinant is +1 (rotation) or -1 (reflection)
    if (!(determinant(mat) > Scalar{ 0 })) {
      return std::nullopt;
    }
    return Rotation{ mat };
  }

  /// @return Rotation C(q) (see rotationMatrix()). q is normalised first, so drift from unit norm
  /// is tolerated. Precondition: q is nonzero and finite.
  static constexpr auto fromQuaternion(const Quaternion<Scalar>& quat) -> Rotation {
    return Rotation{ rotationMatrix(normalized(quat)) };
  }

  /// @return Rotation for frame To = frame From rotated by angle (radians) about x
  static constexpr auto aboutX(Scalar angle) -> Rotation {
    return Rotation{ rx(angle) };
  }

  /// @return Rotation for frame To = frame From rotated by angle (radians) about y
  static constexpr auto aboutY(Scalar angle) -> Rotation {
    return Rotation{ ry(angle) };
  }

  /// @return Rotation for frame To = frame From rotated by angle (radians) about z
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
/// T_AB: maps From (B) relative positions to To (A) relative: _A r_AP = C_AB * _B r_BP + _A r_AB.
/// Default: identity.
template <FrameType To, FrameType From, std::floating_point Scalar = double>
class Transform {
public:
  constexpr Transform() = default;

  /// @param translation _A r_AB: From's origin relative to To's origin, expressed in To
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
