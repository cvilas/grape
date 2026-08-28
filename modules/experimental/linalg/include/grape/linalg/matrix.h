//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <format>
#include <optional>
#include <stdexcept>
#include <utility>

#include "grape/utils/attributes.h"

namespace grape::linalg {

//=================================================================================================
/// Dense matrix with compile-time dimensions.
///
/// Elements are stored in row-major order in a flat std::array. The type is trivially copyable,
/// allocation-free, and usable in constant expressions. Element access is bounds-checked: an
/// out-of-range access throws std::out_of_range at runtime and fails to compile in a constant
/// expression.
///
/// @tparam NumRows Number of rows (> 0)
/// @tparam NumColumns Number of columns (> 0)
/// @tparam Scalar Element type
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar = double>
  requires((NumRows > 0UZ) && (NumColumns > 0UZ) &&
           (std::integral<Scalar> || std::floating_point<Scalar>))
class Matrix {
public:
  /// Creates the zero matrix
  constexpr Matrix() = default;

  /// Creates a matrix from comma-separated element values, specified in row-major order
  template <typename... Values>
    requires((sizeof...(Values) == NumRows * NumColumns) &&
             (std::convertible_to<Values, Scalar> && ...))
  constexpr explicit Matrix(Values... values) : storage_{ static_cast<Scalar>(values)... } {
  }

  /// @return Reference to element at (row, col). Throws std::out_of_range for invalid indices.
  constexpr auto operator[](std::size_t row, std::size_t col) GRAPE_LIFETIMEBOUND->Scalar& {
    return storage_.at(flatIndex(row, col));
  }

  /// @return Reference to element at (row, col). Throws std::out_of_range for invalid indices.
  constexpr auto operator[](std::size_t row,
                            std::size_t col) const GRAPE_LIFETIMEBOUND->const Scalar& {
    return storage_.at(flatIndex(row, col));
  }

  /// @return Reference to element at index (column vectors only)
  constexpr auto operator[](std::size_t index) GRAPE_LIFETIMEBOUND->Scalar&
    requires(NumColumns == 1UZ)
  {
    return storage_.at(index);
  }

  /// @return Reference to element at index (column vectors only)
  constexpr auto operator[](std::size_t index) const GRAPE_LIFETIMEBOUND->const Scalar&
    requires(NumColumns == 1UZ)
  {
    return storage_.at(index);
  }

  /// Exact elementwise equality. For floating point matrices, prefer comparing against a
  /// tolerance instead.
  constexpr auto operator==(const Matrix&) const -> bool = default;

private:
  /// Maps (row, col) to the row-major storage offset. Each index is validated on its own axis: a
  /// col >= NumColumns would otherwise silently alias an element of the next row.
  static constexpr auto flatIndex(std::size_t row, std::size_t col) -> std::size_t {
    if ((row >= NumRows) || (col >= NumColumns)) {
      throw std::out_of_range("Matrix index out of range");
    }
    return (row * NumColumns) + col;
  }

  std::array<Scalar, NumRows * NumColumns> storage_{};
};

//-------------------------------------------------------------------------------------------------
/// Column vector with compile-time length, defined in terms of Matrix
template <std::size_t NumRows, typename Scalar = double>
using Vector = Matrix<NumRows, 1UZ, Scalar>;

namespace detail {

//-------------------------------------------------------------------------------------------------
/// @return (mul1 * mul2) + addend, fused into a single rounding at runtime where supported
template <typename Scalar>
constexpr auto multiplyAdd(Scalar mul1, Scalar mul2, Scalar addend) -> Scalar {
  if consteval {
    return (mul1 * mul2) + addend;
  } else {
    if constexpr (std::floating_point<Scalar>) {
      return std::fma(mul1, mul2, addend);
    } else {
      return (mul1 * mul2) + addend;
    }
  }
}

//-------------------------------------------------------------------------------------------------
/// @return Magnitude of value
template <typename Scalar>
constexpr auto absolute(Scalar value) -> Scalar {
  return (value < Scalar{}) ? -value : value;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return (ax * ay) - (bx * by), evaluated with reduced rounding error using Kahan's algorithm
/// (See https://pharr.org/matt/blog/2019/11/03/difference-of-floats)
template <typename Scalar>
constexpr auto differenceOfProducts(Scalar ax, Scalar ay, Scalar bx, Scalar by) -> Scalar {
  if constexpr (std::floating_point<Scalar>) {
    const auto prod = bx * by;
    const auto rounding_err = detail::multiplyAdd(-bx, by, prod);
    const auto diff = detail::multiplyAdd(ax, ay, -prod);
    return diff + rounding_err;
  } else {
    return (ax * ay) - (bx * by);
  }
}

//-------------------------------------------------------------------------------------------------
/// @return Identity matrix of the specified order
template <std::size_t Order, typename Scalar = double>
constexpr auto identity() -> Matrix<Order, Order, Scalar> {
  auto mat = Matrix<Order, Order, Scalar>{};
  for (std::size_t i = 0UZ; i < Order; ++i) {
    mat[i, i] = Scalar{ 1 };
  }
  return mat;
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise sum lhs + rhs
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
constexpr auto operator+(const Matrix<NumRows, NumColumns, Scalar>& lhs,
                         const Matrix<NumRows, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      result[row, col] = lhs[row, col] + rhs[row, col];
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise negation of mat
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
constexpr auto operator-(const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      result[row, col] = -mat[row, col];
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise difference lhs - rhs
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
constexpr auto operator-(const Matrix<NumRows, NumColumns, Scalar>& lhs,
                         const Matrix<NumRows, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      result[row, col] = lhs[row, col] - rhs[row, col];
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return mat scaled elementwise by factor
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
constexpr auto operator*(const Matrix<NumRows, NumColumns, Scalar>& mat, Scalar factor)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      result[row, col] = mat[row, col] * factor;
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return mat scaled elementwise by factor
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
constexpr auto operator*(Scalar factor, const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return mat * factor;
}

//-------------------------------------------------------------------------------------------------
/// @return Matrix product lhs * rhs
template <std::size_t NumRows, std::size_t Inner, std::size_t NumColumns, typename Scalar>
constexpr auto operator*(const Matrix<NumRows, Inner, Scalar>& lhs,
                         const Matrix<Inner, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  // i-k-j loop order streams both operands row-major, keeping accesses cache-friendly
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t inner = 0UZ; inner < Inner; ++inner) {
      const auto lhs_elem = lhs[row, inner];
      for (std::size_t col = 0UZ; col < NumColumns; ++col) {
        result[row, col] = detail::multiplyAdd(lhs_elem, rhs[inner, col], result[row, col]);
      }
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return Sum of diagonal elements of mat
template <std::size_t Order, typename Scalar>
constexpr auto trace(const Matrix<Order, Order, Scalar>& mat) -> Scalar {
  auto sum = Scalar{};
  for (std::size_t i = 0UZ; i < Order; ++i) {
    sum += mat[i, i];
  }
  return sum;
}

//-------------------------------------------------------------------------------------------------
/// @return Transpose of mat
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
constexpr auto transpose(const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<NumColumns, NumRows, Scalar> {
  auto result = Matrix<NumColumns, NumRows, Scalar>{};
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      result[col, row] = mat[row, col];
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return Copy of the SubRows x SubColumns block of mat whose top-left element is at
/// (StartRow, StartColumn). Bounds are checked at compile time.
template <std::size_t StartRow, std::size_t StartColumn, std::size_t SubRows,
          std::size_t SubColumns, std::size_t NumRows, std::size_t NumColumns, typename Scalar>
  requires((StartRow + SubRows <= NumRows) && (StartColumn + SubColumns <= NumColumns))
constexpr auto submatrix(const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<SubRows, SubColumns, Scalar> {
  auto result = Matrix<SubRows, SubColumns, Scalar>{};
  for (std::size_t row = 0UZ; row < SubRows; ++row) {
    for (std::size_t col = 0UZ; col < SubColumns; ++col) {
      result[row, col] = mat[StartRow + row, StartColumn + col];
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// Overwrites the block of mat whose top-left element is at (StartRow, StartColumn) with sub.
/// Bounds are checked at compile time.
template <std::size_t StartRow, std::size_t StartColumn, std::size_t SubRows,
          std::size_t SubColumns, std::size_t NumRows, std::size_t NumColumns, typename Scalar>
  requires((StartRow + SubRows <= NumRows) && (StartColumn + SubColumns <= NumColumns))
constexpr void setSubmatrix(Matrix<NumRows, NumColumns, Scalar>& mat,
                            const Matrix<SubRows, SubColumns, Scalar>& sub) {
  for (std::size_t row = 0UZ; row < SubRows; ++row) {
    for (std::size_t col = 0UZ; col < SubColumns; ++col) {
      mat[StartRow + row, StartColumn + col] = sub[row, col];
    }
  }
}

//-------------------------------------------------------------------------------------------------
/// @return Determinant of mat, computed by Gaussian elimination with partial pivoting
template <std::size_t Order, std::floating_point Scalar>
constexpr auto determinant(const Matrix<Order, Order, Scalar>& mat) -> Scalar {
  auto lu = mat;
  auto det = Scalar{ 1 };
  for (std::size_t col = 0UZ; col < Order; ++col) {
    auto pivot_row = col;
    auto pivot_mag = detail::absolute(lu[col, col]);
    for (auto row = col + 1UZ; row < Order; ++row) {
      const auto mag = detail::absolute(lu[row, col]);
      if (mag > pivot_mag) {
        pivot_mag = mag;
        pivot_row = row;
      }
    }
    if (!(pivot_mag > Scalar{ 0 })) {
      return Scalar{ 0 };
    }
    if (pivot_row != col) {
      for (std::size_t j = 0UZ; j < Order; ++j) {
        std::swap(lu[col, j], lu[pivot_row, j]);
      }
      det = -det;
    }
    const auto pivot = lu[col, col];
    det *= pivot;
    for (auto row = col + 1UZ; row < Order; ++row) {
      const auto factor = lu[row, col] / pivot;
      for (auto j = col; j < Order; ++j) {
        lu[row, j] = detail::multiplyAdd(-factor, lu[col, j], lu[row, j]);
      }
    }
  }
  return det;
}

//-------------------------------------------------------------------------------------------------
/// @return Multiplicative inverse of mat computed by Gauss-Jordan elimination with partial
/// pivoting, or std::nullopt if mat is singular
template <std::size_t Order, std::floating_point Scalar>
constexpr auto inverse(const Matrix<Order, Order, Scalar>& mat)
    -> std::optional<Matrix<Order, Order, Scalar>> {
  auto lhs = mat;
  auto rhs = identity<Order, Scalar>();
  for (std::size_t col = 0UZ; col < Order; ++col) {
    auto pivot_row = col;
    auto pivot_mag = detail::absolute(lhs[col, col]);
    for (auto row = col + 1UZ; row < Order; ++row) {
      const auto mag = detail::absolute(lhs[row, col]);
      if (mag > pivot_mag) {
        pivot_mag = mag;
        pivot_row = row;
      }
    }
    if (!(pivot_mag > Scalar{ 0 })) {
      return std::nullopt;
    }
    if (pivot_row != col) {
      for (std::size_t j = 0UZ; j < Order; ++j) {
        std::swap(lhs[col, j], lhs[pivot_row, j]);
        std::swap(rhs[col, j], rhs[pivot_row, j]);
      }
    }
    const auto inv_pivot = Scalar{ 1 } / lhs[col, col];
    for (std::size_t j = 0UZ; j < Order; ++j) {
      lhs[col, j] *= inv_pivot;
      rhs[col, j] *= inv_pivot;
    }
    for (std::size_t row = 0UZ; row < Order; ++row) {
      if (row == col) {
        continue;
      }
      const auto factor = lhs[row, col];
      for (std::size_t j = 0UZ; j < Order; ++j) {
        lhs[row, j] = detail::multiplyAdd(-factor, lhs[col, j], lhs[row, j]);
        rhs[row, j] = detail::multiplyAdd(-factor, rhs[col, j], rhs[row, j]);
      }
    }
  }
  return rhs;
}

//-------------------------------------------------------------------------------------------------
/// @return Dot (inner) product of two vectors
template <std::size_t Order, typename Scalar>
constexpr auto dot(const Vector<Order, Scalar>& lhs, const Vector<Order, Scalar>& rhs) -> Scalar {
  auto sum = Scalar{};
  for (std::size_t i = 0UZ; i < Order; ++i) {
    sum = detail::multiplyAdd(lhs[i], rhs[i], sum);
  }
  return sum;
}

//-------------------------------------------------------------------------------------------------
/// @return Euclidean norm (length) of a vector
template <std::size_t Order, std::floating_point Scalar>
constexpr auto norm(const Vector<Order, Scalar>& vec) -> Scalar {
  return std::sqrt(dot(vec, vec));
}

//-------------------------------------------------------------------------------------------------
/// @return Cross product lhs x rhs
template <typename Scalar>
constexpr auto cross(const Vector<3, Scalar>& lhs, const Vector<3, Scalar>& rhs)
    -> Vector<3, Scalar> {
  return Vector<3, Scalar>{
    differenceOfProducts(lhs[1], rhs[2], lhs[2], rhs[1]),
    differenceOfProducts(lhs[2], rhs[0], lhs[0], rhs[2]),
    differenceOfProducts(lhs[0], rhs[1], lhs[1], rhs[0]),
  };
}

}  // namespace grape::linalg

//-------------------------------------------------------------------------------------------------
/// Formatter to support std::format/std::println of matrices. Element format specifiers apply,
/// e.g. std::format("{:.2f}", mat). Rows are separated by ';', e.g. "[[1, 2]; [3, 4]]".
// NOLINTBEGIN(cert-dcl58-cpp)
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
struct std::formatter<grape::linalg::Matrix<NumRows, NumColumns, Scalar>> : std::formatter<Scalar> {
  auto format(const grape::linalg::Matrix<NumRows, NumColumns, Scalar>& mat,
              std::format_context& ctx) const {
    auto out = ctx.out();
    out = std::format_to(out, "[");
    for (std::size_t row = 0UZ; row < NumRows; ++row) {
      out = std::format_to(out, "[");
      for (std::size_t col = 0UZ; col < NumColumns; ++col) {
        ctx.advance_to(out);
        out = std::formatter<Scalar>::format(mat[row, col], ctx);
        if (col + 1UZ < NumColumns) {
          out = std::format_to(out, ", ");
        }
      }
      out = std::format_to(out, "]");
      if (row + 1UZ < NumRows) {
        out = std::format_to(out, "; ");
      }
    }
    return std::format_to(out, "]");
  }
};
// NOLINTEND(cert-dcl58-cpp)
