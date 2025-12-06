//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <concepts>
#include <type_traits>

namespace grape::linalg {

template <std::size_t NumRows, std::size_t NumColumns, typename Scalar = double>
  requires std::integral<Scalar> || std::floating_point<Scalar>
class Matrix {
public:
  constexpr auto operator[](std::size_t row, std::size_t col) -> Scalar& {
    return storage_[(row * NumColumns) + col];
  }

  constexpr auto operator[](std::size_t row, std::size_t col) const -> const Scalar& {
    return storage_[(row * NumColumns) + col];
  }

  // todo - make this clear
  template <std::size_t NumRows2, std::size_t NumColumns2>
  constexpr auto operator*(const Matrix<NumRows2, NumColumns2, Scalar>& m2) {
    (void)m2;
    // See: https://youtu.be/XzUTLsWyErA
    // - blocking + tiling + hoisting
    // - transpose second matrix then dot product rows
    // - tune parameters for best performance with 3x3, 4x4, 6x6 matrices
    // - strassen's algorithm
    // This could be a free function taking in matrix 'views' making block multiplication easier to
    // implement
  }
  // todo:
  // - views (span and mdspan) to avoid copies
  // - comma initialisation
  // - dot product free function of two vectors views
  // - vector product of two views
  // - multiply_add free function
  // - matrix multiply over matrix views (large matmult can be implemented as block mult_add easily)
private:
  std::array<Scalar, NumRows * NumColumns> storage_{};
};

template <std::size_t order, typename Scalar>
using Vector = Matrix<order, 1UZ, Scalar>;

//-------------------------------------------------------------------------------------------------
template <std::size_t order, typename Scalar>
constexpr auto identity() -> Matrix<order, order, Scalar> {
  auto mat = Matrix<order, order, Scalar>{};
  for (std::size_t i = 0; i < order; ++i) {
    mat[i, i] = static_cast<Scalar>(1);
  }
  return mat;
}

//-------------------------------------------------------------------------------------------------
template <typename Scalar>
constexpr auto cross(const Vector<3, Scalar>& /*va*/, const Vector<3, Scalar>& /*vb*/)
    -> Vector<3, Scalar> {
  // use kahan's approach: https://pharr.org/matt/blog/2019/11/03/difference-of-floats
  return Vector<3, Scalar>{ 1, 2, 3 };
}

}  // namespace grape::linalg
