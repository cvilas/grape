//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "grape/utils/attributes.h"

// Loop unrolling hints for GCC only (see detail::multiply). Undefined at the end of this file.
// - GRAPE_LINALG_GCC_UNROLL: unroll by 8, which fully unrolls loops over small dimensions
// - GRAPE_LINALG_GCC_UNROLL_FULL: fully unroll loops of up to 16 iterations, so that small local
//   arrays indexed by them can be held in registers
// - GRAPE_LINALG_GCC_UNROLL_ROWS: unroll by the RowUnroll template parameter of the enclosing
//   function (GCC accepts only a literal or a template parameter name as the count)
#if defined(__GNUC__) && !defined(__clang__)
#define GRAPE_LINALG_GCC_UNROLL _Pragma("GCC unroll 8")
#define GRAPE_LINALG_GCC_UNROLL_FULL _Pragma("GCC unroll 16")
#define GRAPE_LINALG_GCC_UNROLL_ROWS _Pragma("GCC unroll RowUnroll")
#else
#define GRAPE_LINALG_GCC_UNROLL
#define GRAPE_LINALG_GCC_UNROLL_FULL
#define GRAPE_LINALG_GCC_UNROLL_ROWS
#endif

namespace grape::linalg {

//=================================================================================================
/// Containers operate on numeric types
template <typename T>
concept NumericType = std::floating_point<T> ||
                      (std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char> &&
                       !std::same_as<T, wchar_t> && !std::same_as<T, char8_t> &&
                       !std::same_as<T, char16_t> && !std::same_as<T, char32_t>);

//=================================================================================================
/// Dense matrix with compile-time dimensions.
///
/// - Elements are stored flat in row-major order.
/// - Trivially copyable, allocation-free, and usable in constant expressions.
/// - Element access is bounds-checked (at compile time for constant expressions).
///
/// @tparam NumRows Number of rows (> 0)
/// @tparam NumColumns Number of columns (> 0)
/// @tparam Scalar Element type (see NumericType)
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar = double>
  requires((NumRows > 0UZ) && (NumColumns > 0UZ))
class Matrix {
public:
  /// Number of elements
  static constexpr auto SIZE = NumRows * NumColumns;

  /// Creates the zero matrix
  constexpr Matrix() = default;

  /// Creates a matrix from comma-separated element values, specified in row-major order.
  template <typename... Values>
    requires((sizeof...(Values) == SIZE) && (std::convertible_to<Values, Scalar> && ...) &&
             (std::floating_point<Scalar> || (std::integral<Values> && ...)))
  constexpr explicit Matrix(Values... values) : storage_{ static_cast<Scalar>(values)... } {
  }

  /// @return Reference to element at (row, col). Throws std::out_of_range for invalid indices.
  constexpr auto operator[](std::size_t row, std::size_t col) GRAPE_LIFETIMEBOUND->Scalar& {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    return storage_[flatIndex(row, col)];
  }

  /// @return Reference to element at (row, col). Throws std::out_of_range for invalid indices.
  constexpr auto operator[](std::size_t row,
                            std::size_t col) const GRAPE_LIFETIMEBOUND->const Scalar& {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    return storage_[flatIndex(row, col)];
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

  /// @return All elements as a contiguous array in row-major order, for whole-matrix algorithms
  /// and interoperability with other libraries
  [[nodiscard]] constexpr auto elements() GRAPE_LIFETIMEBOUND -> std::array<Scalar, SIZE>& {
    return storage_;
  }

  /// @return All elements as a contiguous array in row-major order
  [[nodiscard]] constexpr auto elements() const GRAPE_LIFETIMEBOUND
      -> const std::array<Scalar, SIZE>& {
    return storage_;
  }

private:
  static constexpr auto flatIndex(std::size_t row, std::size_t col) -> std::size_t {
    if ((row >= NumRows) || (col >= NumColumns)) {
      throw std::out_of_range("Matrix index out of range");
    }
    return (row * NumColumns) + col;
  }

  std::array<Scalar, SIZE> storage_{};
};

//-------------------------------------------------------------------------------------------------
/// Column vector with compile-time length
template <std::size_t NumRows, NumericType Scalar = double>
using Vector = Matrix<NumRows, 1UZ, Scalar>;

namespace detail {

//-------------------------------------------------------------------------------------------------
/// @return (mul1 * mul2) + addend, fused into a single rounding for floating point types.
template <NumericType Scalar>
constexpr auto multiplyAdd(Scalar mul1, Scalar mul2, Scalar addend) -> Scalar {
  if constexpr (std::floating_point<Scalar>) {
#ifdef __clang__
    if consteval {
      return (mul1 * mul2) + addend;
    }
#endif
    return std::fma(mul1, mul2, addend);
  } else {
    return (mul1 * mul2) + addend;
  }
}

//-------------------------------------------------------------------------------------------------
/// @return Magnitude of value
template <NumericType Scalar>
constexpr auto absolute(Scalar value) -> Scalar {
  return (value < Scalar{}) ? -value : value;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return (ax * ay) - (bx * by), evaluated with reduced rounding error using Kahan's algorithm
/// (See ./docs/kahans_algorithm.md)
template <NumericType Scalar>
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
template <std::size_t Order, NumericType Scalar = double>
constexpr auto identity() -> Matrix<Order, Order, Scalar> {
  auto mat = Matrix<Order, Order, Scalar>{};
  for (std::size_t i = 0UZ; i < Order; ++i) {
    mat[i, i] = Scalar{ 1 };
  }
  return mat;
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise sum lhs + rhs
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator+(const Matrix<NumRows, NumColumns, Scalar>& lhs,
                         const Matrix<NumRows, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  auto& out = result.elements();
  GRAPE_LINALG_GCC_UNROLL
  for (std::size_t i = 0UZ; i < out.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    out[i] = lhs.elements()[i] + rhs.elements()[i];
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise negation of mat
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator-(const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  auto& out = result.elements();
  GRAPE_LINALG_GCC_UNROLL
  for (std::size_t i = 0UZ; i < out.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    out[i] = -mat.elements()[i];
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise difference lhs - rhs
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator-(const Matrix<NumRows, NumColumns, Scalar>& lhs,
                         const Matrix<NumRows, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  auto& out = result.elements();
  GRAPE_LINALG_GCC_UNROLL
  for (std::size_t i = 0UZ; i < out.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    out[i] = lhs.elements()[i] - rhs.elements()[i];
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return mat scaled elementwise by factor
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator*(const Matrix<NumRows, NumColumns, Scalar>& mat,
                         std::type_identity_t<Scalar> factor)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  auto& out = result.elements();
  GRAPE_LINALG_GCC_UNROLL
  for (std::size_t i = 0UZ; i < out.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    out[i] = mat.elements()[i] * factor;
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return mat scaled elementwise by factor
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator*(std::type_identity_t<Scalar> factor,
                         const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return mat * factor;
}

//-------------------------------------------------------------------------------------------------
/// @return mat divided elementwise by divisor (truncating for integral matrices)
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator/(const Matrix<NumRows, NumColumns, Scalar>& mat,
                         std::type_identity_t<Scalar> divisor)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  auto& out = result.elements();
  GRAPE_LINALG_GCC_UNROLL
  for (std::size_t i = 0UZ; i < out.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    out[i] = mat.elements()[i] / divisor;
  }
  return result;
}

namespace detail {

/// Largest dimension for which detail::multiply fully unrolls its outer loops
inline constexpr auto SMALL_KERNEL_SIZE = 8UZ;

//-------------------------------------------------------------------------------------------------
/// @return Matrix product lhs * rhs
///
/// i-k-j loop order streams both operands row-major. Two kernels, chosen by size, because GCC
/// (unlike Clang) needs the shape of the code to suit its optimiser. Clang ignores the hints and
/// is fast with either kernel. See docs/benchmarking/benchmarking.md.
/// - Small (rows and inner dimension <= 8): the two outer loops are fully unrolled, which lets GCC
///   keep the whole result in registers and vectorise the contiguous innermost loop.
/// - Larger: each result row is accumulated in a local array, and the column loop is fully
///   unrolled so the array is held in registers. Accumulating directly into the result makes
///   every step store and reload it, a dependency chain through memory (3.6x slower at 15x15).
///   Unrolling the row loop as well helps only up to 8 rows, hence RowUnroll.
template <std::size_t NumRows, std::size_t Inner, std::size_t NumColumns, NumericType Scalar,
          std::size_t RowUnroll = ((NumRows <= SMALL_KERNEL_SIZE) ? SMALL_KERNEL_SIZE : 1UZ)>
constexpr auto multiply(const Matrix<NumRows, Inner, Scalar>& lhs,
                        const Matrix<Inner, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  if constexpr ((NumRows <= SMALL_KERNEL_SIZE) && (Inner <= SMALL_KERNEL_SIZE)) {
    GRAPE_LINALG_GCC_UNROLL
    for (std::size_t row = 0UZ; row < NumRows; ++row) {
      GRAPE_LINALG_GCC_UNROLL
      for (std::size_t inner = 0UZ; inner < Inner; ++inner) {
        const auto lhs_elem = lhs[row, inner];
        for (std::size_t col = 0UZ; col < NumColumns; ++col) {
          result[row, col] = multiplyAdd(lhs_elem, rhs[inner, col], result[row, col]);
        }
      }
    }
    return result;
  }
  GRAPE_LINALG_GCC_UNROLL_ROWS
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    auto acc = std::array<Scalar, NumColumns>{};
    GRAPE_LINALG_GCC_UNROLL
    for (std::size_t inner = 0UZ; inner < Inner; ++inner) {
      const auto lhs_elem = lhs[row, inner];
      GRAPE_LINALG_GCC_UNROLL_FULL
      for (std::size_t col = 0UZ; col < NumColumns; ++col) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
        acc[col] = multiplyAdd(lhs_elem, rhs[inner, col], acc[col]);
      }
    }
    GRAPE_LINALG_GCC_UNROLL_FULL
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
      result[row, col] = acc[col];
    }
  }
  return result;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return Matrix product lhs * rhs
template <std::size_t NumRows, std::size_t Inner, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator*(const Matrix<NumRows, Inner, Scalar>& lhs,
                         const Matrix<Inner, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return detail::multiply(lhs, rhs);
}

//-------------------------------------------------------------------------------------------------
/// @return Sum of diagonal elements of mat
template <std::size_t Order, NumericType Scalar>
constexpr auto trace(const Matrix<Order, Order, Scalar>& mat) -> Scalar {
  auto sum = Scalar{};
  for (std::size_t i = 0UZ; i < Order; ++i) {
    sum += mat[i, i];
  }
  return sum;
}

//-------------------------------------------------------------------------------------------------
/// @return Transpose of mat
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
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
/// (StartRow, StartColumn).
template <std::size_t StartRow, std::size_t StartColumn, std::size_t SubRows,
          std::size_t SubColumns, std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
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
template <std::size_t StartRow, std::size_t StartColumn, std::size_t SubRows,
          std::size_t SubColumns, std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
  requires((StartRow + SubRows <= NumRows) && (StartColumn + SubColumns <= NumColumns))
constexpr void setSubmatrix(Matrix<NumRows, NumColumns, Scalar>& mat,
                            const Matrix<SubRows, SubColumns, Scalar>& sub) {
  for (std::size_t row = 0UZ; row < SubRows; ++row) {
    for (std::size_t col = 0UZ; col < SubColumns; ++col) {
      mat[StartRow + row, StartColumn + col] = sub[row, col];
    }
  }
}

namespace detail {

//-------------------------------------------------------------------------------------------------
/// @return Row index at or below the diagonal of column col holding the largest magnitude element
/// (the partial pivot), and that magnitude
template <std::size_t Order, NumericType Scalar>
constexpr auto findPivot(const Matrix<Order, Order, Scalar>& mat, std::size_t col)
    -> std::pair<std::size_t, Scalar> {
  auto pivot_row = col;
  auto pivot_mag = absolute(mat[col, col]);
  for (auto row = col + 1UZ; row < Order; ++row) {
    const auto mag = absolute(mat[row, col]);
    if (mag > pivot_mag) {
      pivot_mag = mag;
      pivot_row = row;
    }
  }
  return { pivot_row, pivot_mag };
}

//-------------------------------------------------------------------------------------------------
/// Swaps rows row_a and row_b of mat, from column first_col onwards
/// (the row arguments are interchangeable, so swapping them by mistake is harmless)
template <std::size_t Order, NumericType Scalar>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr void swapRows(Matrix<Order, Order, Scalar>& mat, std::size_t row_a, std::size_t row_b,
                        std::size_t first_col = 0UZ) {
  for (auto j = first_col; j < Order; ++j) {
    std::swap(mat[row_a, j], mat[row_b, j]);
  }
}

//-------------------------------------------------------------------------------------------------
/// @return Infinity norm of mat: the largest absolute row sum
template <std::size_t Order, NumericType Scalar>
constexpr auto infinityNorm(const Matrix<Order, Order, Scalar>& mat) -> Scalar {
  auto result = Scalar{ 0 };
  for (std::size_t row = 0UZ; row < Order; ++row) {
    auto row_sum = Scalar{ 0 };
    for (std::size_t col = 0UZ; col < Order; ++col) {
      row_sum += absolute(mat[row, col]);
    }
    result = (row_sum > result) ? row_sum : result;
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
/// @return (x1 * y1) + (x2 * y2) + (x3 * y3), with fused accumulation
template <NumericType Scalar>
constexpr auto sumOfThreeProducts(Scalar x1, Scalar y1, Scalar x2, Scalar y2, Scalar x3, Scalar y3)
    -> Scalar {
  return multiplyAdd(x1, y1, multiplyAdd(x2, y2, x3 * y3));
}

//-------------------------------------------------------------------------------------------------
/// Adjugate (transposed cofactor matrix) and determinant of a small matrix, from which the inverse
/// is adjugate / determinant
template <std::size_t Order, NumericType Scalar>
struct Adjugate {
  Matrix<Order, Order, Scalar> adjugate;
  Scalar determinant;
};

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
constexpr auto adjugate(const Matrix<2, 2, Scalar>& mat) -> Adjugate<2, Scalar> {
  return {
    .adjugate = Matrix<2, 2, Scalar>{ mat[1, 1], -mat[0, 1], -mat[1, 0], mat[0, 0] },
    .determinant = differenceOfProducts(mat[0, 0], mat[1, 1], mat[0, 1], mat[1, 0]),
  };
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
constexpr auto adjugate(const Matrix<3, 3, Scalar>& mat) -> Adjugate<3, Scalar> {
  // cofactors c_ij; each is a 2x2 determinant computed with Kahan's difference of products
  const auto c00 = differenceOfProducts(mat[1, 1], mat[2, 2], mat[1, 2], mat[2, 1]);
  const auto c01 = differenceOfProducts(mat[1, 2], mat[2, 0], mat[1, 0], mat[2, 2]);
  const auto c02 = differenceOfProducts(mat[1, 0], mat[2, 1], mat[1, 1], mat[2, 0]);
  const auto c10 = differenceOfProducts(mat[0, 2], mat[2, 1], mat[0, 1], mat[2, 2]);
  const auto c11 = differenceOfProducts(mat[0, 0], mat[2, 2], mat[0, 2], mat[2, 0]);
  const auto c12 = differenceOfProducts(mat[0, 1], mat[2, 0], mat[0, 0], mat[2, 1]);
  const auto c20 = differenceOfProducts(mat[0, 1], mat[1, 2], mat[0, 2], mat[1, 1]);
  const auto c21 = differenceOfProducts(mat[0, 2], mat[1, 0], mat[0, 0], mat[1, 2]);
  const auto c22 = differenceOfProducts(mat[0, 0], mat[1, 1], mat[0, 1], mat[1, 0]);
  return {
    .adjugate = Matrix<3, 3, Scalar>{ c00, c10, c20, c01, c11, c21, c02, c12, c22 },
    .determinant = multiplyAdd(mat[0, 0], c00, multiplyAdd(mat[0, 1], c01, mat[0, 2] * c02)),
  };
}

//-------------------------------------------------------------------------------------------------
/// Laplace expansion along the top two rows: the 2x2 minors of rows 0-1 (upper) and rows 2-3
/// (lower) are shared by every cofactor, so 12 two-by-two determinants yield the whole adjugate.
template <std::floating_point Scalar>
constexpr auto adjugate(const Matrix<4, 4, Scalar>& mat) -> Adjugate<4, Scalar> {
  // upper[i]: minors of rows 0-1, lower[i]: complementary minors of rows 2-3
  const auto up0 = differenceOfProducts(mat[0, 0], mat[1, 1], mat[0, 1], mat[1, 0]);
  const auto up1 = differenceOfProducts(mat[0, 0], mat[1, 2], mat[0, 2], mat[1, 0]);
  const auto up2 = differenceOfProducts(mat[0, 0], mat[1, 3], mat[0, 3], mat[1, 0]);
  const auto up3 = differenceOfProducts(mat[0, 1], mat[1, 2], mat[0, 2], mat[1, 1]);
  const auto up4 = differenceOfProducts(mat[0, 1], mat[1, 3], mat[0, 3], mat[1, 1]);
  const auto up5 = differenceOfProducts(mat[0, 2], mat[1, 3], mat[0, 3], mat[1, 2]);
  const auto lo0 = differenceOfProducts(mat[2, 0], mat[3, 1], mat[2, 1], mat[3, 0]);
  const auto lo1 = differenceOfProducts(mat[2, 0], mat[3, 2], mat[2, 2], mat[3, 0]);
  const auto lo2 = differenceOfProducts(mat[2, 0], mat[3, 3], mat[2, 3], mat[3, 0]);
  const auto lo3 = differenceOfProducts(mat[2, 1], mat[3, 2], mat[2, 2], mat[3, 1]);
  const auto lo4 = differenceOfProducts(mat[2, 1], mat[3, 3], mat[2, 3], mat[3, 1]);
  const auto lo5 = differenceOfProducts(mat[2, 2], mat[3, 3], mat[2, 3], mat[3, 2]);
  const auto det = multiplyAdd(
      up0, lo5,
      multiplyAdd(-up1, lo4,
                  multiplyAdd(up2, lo3, multiplyAdd(up3, lo2, multiplyAdd(-up4, lo1, up5 * lo0)))));
  return {
    .adjugate =
        Matrix<4, 4, Scalar>{
            sumOfThreeProducts(mat[1, 1], lo5, -mat[1, 2], lo4, mat[1, 3], lo3),
            sumOfThreeProducts(-mat[0, 1], lo5, mat[0, 2], lo4, -mat[0, 3], lo3),
            sumOfThreeProducts(mat[3, 1], up5, -mat[3, 2], up4, mat[3, 3], up3),
            sumOfThreeProducts(-mat[2, 1], up5, mat[2, 2], up4, -mat[2, 3], up3),
            sumOfThreeProducts(-mat[1, 0], lo5, mat[1, 2], lo2, -mat[1, 3], lo1),
            sumOfThreeProducts(mat[0, 0], lo5, -mat[0, 2], lo2, mat[0, 3], lo1),
            sumOfThreeProducts(-mat[3, 0], up5, mat[3, 2], up2, -mat[3, 3], up1),
            sumOfThreeProducts(mat[2, 0], up5, -mat[2, 2], up2, mat[2, 3], up1),
            sumOfThreeProducts(mat[1, 0], lo4, -mat[1, 1], lo2, mat[1, 3], lo0),
            sumOfThreeProducts(-mat[0, 0], lo4, mat[0, 1], lo2, -mat[0, 3], lo0),
            sumOfThreeProducts(mat[3, 0], up4, -mat[3, 1], up2, mat[3, 3], up0),
            sumOfThreeProducts(-mat[2, 0], up4, mat[2, 1], up2, -mat[2, 3], up0),
            sumOfThreeProducts(-mat[1, 0], lo3, mat[1, 1], lo1, -mat[1, 2], lo0),
            sumOfThreeProducts(mat[0, 0], lo3, -mat[0, 1], lo1, mat[0, 2], lo0),
            sumOfThreeProducts(-mat[3, 0], up3, mat[3, 1], up1, -mat[3, 2], up0),
            sumOfThreeProducts(mat[2, 0], up3, -mat[2, 1], up1, mat[2, 2], up0),
        },
    .determinant = det,
  };
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return Determinant of mat. Orders 1-4 use closed forms; larger orders use Gaussian elimination
/// with partial pivoting, returning exactly 0 if a pivot is exactly zero.
template <std::size_t Order, std::floating_point Scalar>
constexpr auto determinant(const Matrix<Order, Order, Scalar>& mat) -> Scalar {
  if constexpr (Order == 1UZ) {
    return mat[0, 0];
  } else if constexpr (Order == 2UZ) {
    return differenceOfProducts(mat[0, 0], mat[1, 1], mat[0, 1], mat[1, 0]);
  } else if constexpr (Order == 3UZ) {
    const auto minor0 = differenceOfProducts(mat[1, 1], mat[2, 2], mat[1, 2], mat[2, 1]);
    const auto minor1 = differenceOfProducts(mat[1, 0], mat[2, 2], mat[1, 2], mat[2, 0]);
    const auto minor2 = differenceOfProducts(mat[1, 0], mat[2, 1], mat[1, 1], mat[2, 0]);
    return detail::multiplyAdd(mat[0, 0], minor0,
                               detail::multiplyAdd(-mat[0, 1], minor1, mat[0, 2] * minor2));
  } else if constexpr (Order == 4UZ) {
    return detail::adjugate(mat).determinant;
  } else {
    auto lu = mat;
    auto det = Scalar{ 1 };
    for (std::size_t col = 0UZ; col < Order; ++col) {
      const auto [pivot_row, pivot_mag] = detail::findPivot(lu, col);
      if (!(pivot_mag > Scalar{ 0 })) {
        return Scalar{ 0 };
      }
      if (pivot_row != col) {
        detail::swapRows(lu, col, pivot_row, col);
        det = -det;
      }
      const auto pivot = lu[col, col];
      det *= pivot;
      // column col below the diagonal is never read again, so only update columns to its right
      for (auto row = col + 1UZ; row < Order; ++row) {
        const auto factor = lu[row, col] / pivot;
        for (auto j = col + 1UZ; j < Order; ++j) {
          lu[row, j] = detail::multiplyAdd(-factor, lu[col, j], lu[row, j]);
        }
      }
    }
    return det;
  }
}

namespace detail {

//-------------------------------------------------------------------------------------------------
/// @return Inverse of mat by Gauss-Jordan elimination with partial pivoting, or std::nullopt if a
/// pivot is at most Order * epsilon * mat_norm (see inverse())
template <std::size_t Order, std::floating_point Scalar>
constexpr auto inverseGaussJordan(const Matrix<Order, Order, Scalar>& mat, Scalar mat_norm)
    -> std::optional<Matrix<Order, Order, Scalar>> {
  const auto singular_tolerance =
      static_cast<Scalar>(Order) * std::numeric_limits<Scalar>::epsilon() * mat_norm;

  // Reduce lhs to the identity; applying the same row operations to rhs turns it into the inverse.
  // Rows are updated in full: columns of lhs left of col are already unit columns (zero in the
  // pivot row) so updating them changes nothing, but a constant trip count vectorises better.
  auto lhs = mat;
  auto rhs = identity<Order, Scalar>();
  for (std::size_t col = 0UZ; col < Order; ++col) {
    const auto [pivot_row, pivot_mag] = findPivot(lhs, col);
    // written so that NaN (all comparisons false) is also rejected
    if (!(pivot_mag > singular_tolerance)) {
      return std::nullopt;
    }
    if (pivot_row != col) {
      swapRows(lhs, col, pivot_row, col);
      swapRows(rhs, col, pivot_row);
    }
    // The pivot row is scaled into local copies. Reading from a separate array lets GCC vectorise
    // the row updates without having to prove that row and col are different rows (1.7x faster
    // at 15x15).
    const auto inv_pivot = Scalar{ 1 } / lhs[col, col];
    auto pivot_lhs = std::array<Scalar, Order>{};
    auto pivot_rhs = std::array<Scalar, Order>{};
    for (std::size_t j = 0UZ; j < Order; ++j) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
      pivot_lhs[j] = lhs[col, j] * inv_pivot;
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
      lhs[col, j] = pivot_lhs[j];
    }
    for (std::size_t j = 0UZ; j < Order; ++j) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
      pivot_rhs[j] = rhs[col, j] * inv_pivot;
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
      rhs[col, j] = pivot_rhs[j];
    }
    for (std::size_t row = 0UZ; row < Order; ++row) {
      if (row == col) {
        continue;
      }
      const auto factor = lhs[row, col];
      for (std::size_t j = 0UZ; j < Order; ++j) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
        lhs[row, j] = multiplyAdd(-factor, pivot_lhs[j], lhs[row, j]);
      }
      for (std::size_t j = 0UZ; j < Order; ++j) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
        rhs[row, j] = multiplyAdd(-factor, pivot_rhs[j], rhs[row, j]);
      }
    }
  }
  return rhs;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return Multiplicative inverse of mat, or std::nullopt if mat is singular to working precision.
///
/// Singularity is defined by Gauss-Jordan elimination with partial pivoting: mat is singular when a
/// pivot falls to within rounding error of zero, relative to the scale of mat:
/// |pivot| <= n * epsilon * ||mat||_inf. Exact zero pivots are rare in floating point, so an
/// absolute test would let numerically singular matrices through and return an inverse made of
/// amplified rounding noise. Being relative to the whole matrix, the test also rejects badly
/// scaled matrices (e.g. diag(1, 1e-300)); rescale rows/columns to comparable magnitudes (e.g.
/// consistent units) before inverting.
///
/// Orders 2-4 normally use the closed form adjugate / determinant, several times faster than
/// elimination. It is taken only when it provably agrees with elimination about singularity: with
/// partial pivoting every pivot satisfies |u_kk| <= 2^(k-1) * ||mat||_inf, so if any pivot were at
/// most the tolerance, then |det| = prod |u_kk| <= n * epsilon * 2^(n(n-1)/2) * ||mat||_inf^n. A
/// determinant above that bound therefore guarantees elimination would also succeed. Below it, or
/// when the bound under/overflows, elimination decides.
template <std::size_t Order, std::floating_point Scalar>
constexpr auto inverse(const Matrix<Order, Order, Scalar>& mat)
    -> std::optional<Matrix<Order, Order, Scalar>> {
  const auto mat_norm = detail::infinityNorm(mat);
  if constexpr ((Order >= 2UZ) && (Order <= 4UZ)) {
    constexpr auto PIVOT_GROWTH = static_cast<Scalar>(1ULL << (Order * (Order - 1UZ) / 2UZ));
    auto norm_power = Scalar{ 1 };
    for (std::size_t i = 0UZ; i < Order; ++i) {
      norm_power *= mat_norm;
    }
    const auto det_bound = static_cast<Scalar>(Order) * std::numeric_limits<Scalar>::epsilon() *
                           PIVOT_GROWTH * norm_power;
    const auto [adj, det] = detail::adjugate(mat);
    if ((det_bound >= std::numeric_limits<Scalar>::min()) && (detail::absolute(det) > det_bound)) {
      return adj * (Scalar{ 1 } / det);
    }
  }
  return detail::inverseGaussJordan(mat, mat_norm);
}

//-------------------------------------------------------------------------------------------------
/// @return Dot (inner) product of two vectors
template <std::size_t Order, NumericType Scalar>
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
template <NumericType Scalar>
constexpr auto cross(const Vector<3, Scalar>& lhs, const Vector<3, Scalar>& rhs)
    -> Vector<3, Scalar> {
  return Vector<3, Scalar>{
    differenceOfProducts(lhs[1], rhs[2], lhs[2], rhs[1]),
    differenceOfProducts(lhs[2], rhs[0], lhs[0], rhs[2]),
    differenceOfProducts(lhs[0], rhs[1], lhs[1], rhs[0]),
  };
}

}  // namespace grape::linalg

#undef GRAPE_LINALG_GCC_UNROLL
#undef GRAPE_LINALG_GCC_UNROLL_FULL
#undef GRAPE_LINALG_GCC_UNROLL_ROWS

//-------------------------------------------------------------------------------------------------
/// Formatter to support std::format/std::println of matrices. Element format specifiers apply,
/// e.g. std::format("{:.2f}", mat). Rows are separated by ';', e.g. "[[1, 2]; [3, 4]]".
// NOLINTBEGIN(cert-dcl58-cpp)
template <std::size_t NumRows, std::size_t NumColumns, grape::linalg::NumericType Scalar>
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
