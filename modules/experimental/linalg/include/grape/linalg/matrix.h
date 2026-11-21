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

// GCC vectorises small loops only when they are unrolled (see docs/benchmarking). Clang does not
// need these hints. GCC takes only a literal or a template parameter name as the unroll count.
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

namespace detail {
/// Kept out of line: an inline throw makes GCC stop inlining element access in large functions.
[[noreturn, gnu::cold, gnu::noinline]] inline void throwOutOfRange(const char* what) {
  throw std::out_of_range(what);
}
}  // namespace detail

//=================================================================================================
/// Arithmetic element types, excluding bool and character types
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

  /// @return Reference to element at index (column vectors only). Throws std::out_of_range for
  /// invalid indices.
  constexpr auto operator[](std::size_t index) GRAPE_LIFETIMEBOUND->Scalar&
    requires(NumColumns == 1UZ)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    return storage_[flatIndex(index, 0UZ)];
  }

  /// @return Reference to element at index (column vectors only). Throws std::out_of_range for
  /// invalid indices.
  constexpr auto operator[](std::size_t index) const GRAPE_LIFETIMEBOUND->const Scalar&
    requires(NumColumns == 1UZ)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    return storage_[flatIndex(index, 0UZ)];
  }

  /// Exact elementwise equality. For floating point matrices, prefer comparing against a
  /// tolerance instead.
  constexpr auto operator==(const Matrix&) const -> bool = default;

  /// @return All elements, contiguous and row-major (e.g. to pass to other libraries)
  [[nodiscard]] constexpr auto elements() GRAPE_LIFETIMEBOUND -> std::array<Scalar, SIZE>& {
    return storage_;
  }

  [[nodiscard]] constexpr auto elements() const GRAPE_LIFETIMEBOUND
      -> const std::array<Scalar, SIZE>& {
    return storage_;
  }

private:
  static constexpr auto flatIndex(std::size_t row, std::size_t col) -> std::size_t {
    if ((row >= NumRows) || (col >= NumColumns)) {
      detail::throwOutOfRange("Matrix index out of range");
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
/// @return (mul1 * mul2) + addend, rounded once for floating point types
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
template <NumericType Scalar>
constexpr auto absolute(Scalar value) -> Scalar {
  return (value < Scalar{}) ? -value : value;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return (ax * ay) - (bx * by), accurate to a few ulp even under cancellation (Kahan's algorithm,
/// see docs/kahan.pdf)
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

namespace detail {

//-------------------------------------------------------------------------------------------------
/// @return Matrix of op applied to corresponding elements.
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar, typename Op,
          std::same_as<Matrix<NumRows, NumColumns, Scalar>>... Mats>
constexpr auto elementwise(Op op, const Matrix<NumRows, NumColumns, Scalar>& first,
                           const Mats&... rest) -> Matrix<NumRows, NumColumns, Scalar> {
  auto result = Matrix<NumRows, NumColumns, Scalar>{};
  auto& out = result.elements();
  // GCC optimises better over indexed loop instead of std::ranges::transform
  GRAPE_LINALG_GCC_UNROLL
  for (std::size_t i = 0UZ; i < out.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,cppcoreguidelines-pro-bounds-constant-array-index)
    out[i] = op(first.elements()[i], rest.elements()[i]...);
  }
  return result;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return Elementwise sum lhs + rhs
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator+(const Matrix<NumRows, NumColumns, Scalar>& lhs,
                         const Matrix<NumRows, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return detail::elementwise(std::plus{}, lhs, rhs);
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise negation of mat
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator-(const Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return detail::elementwise(std::negate{}, mat);
}

//-------------------------------------------------------------------------------------------------
/// @return Elementwise difference lhs - rhs
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator-(const Matrix<NumRows, NumColumns, Scalar>& lhs,
                         const Matrix<NumRows, NumColumns, Scalar>& rhs)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return detail::elementwise(std::minus{}, lhs, rhs);
}

//-------------------------------------------------------------------------------------------------
/// @return mat scaled elementwise by factor
template <std::size_t NumRows, std::size_t NumColumns, NumericType Scalar>
constexpr auto operator*(const Matrix<NumRows, NumColumns, Scalar>& mat,
                         std::type_identity_t<Scalar> factor)
    -> Matrix<NumRows, NumColumns, Scalar> {
  return detail::elementwise([factor](Scalar elem) { return elem * factor; }, mat);
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
  return detail::elementwise([divisor](Scalar elem) { return elem / divisor; }, mat);
}

namespace detail {

inline constexpr auto SMALL_KERNEL_SIZE = 8UZ;

//-------------------------------------------------------------------------------------------------
/// @return Matrix product lhs * rhs
///
/// The loops run in i-k-j order so that both operands are read row-major. There are two kernels
/// because GCC needs different code shapes to vectorise small and large sizes (Clang is fast with
/// either):
/// - Small: the outer loops are unrolled so the whole result stays in registers.
/// - Large: each result row is accumulated in a local array held in registers. Accumulating into
///   the result instead stores and reloads it on every step.
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
/// @return Copy of the SubRows x SubColumns block of mat starting at (StartRow, StartColumn)
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
/// Overwrites the block of mat starting at (StartRow, StartColumn) with sub
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
/// @return Row and magnitude of the largest element in column col, on or below the diagonal
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
template <std::size_t Order, NumericType Scalar>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr void swapRows(Matrix<Order, Order, Scalar>& mat, std::size_t row_a, std::size_t row_b,
                        std::size_t first_col = 0UZ) {
  for (auto j = first_col; j < Order; ++j) {
    std::swap(mat[row_a, j], mat[row_b, j]);
  }
}

//-------------------------------------------------------------------------------------------------
/// @return Infinity norm of mat (largest absolute row sum)
template <std::size_t Order, NumericType Scalar>
GRAPE_ALWAYS_INLINE constexpr auto infinityNorm(const Matrix<Order, Order, Scalar>& mat) -> Scalar {
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
/// @return true if no element is inf or NaN
template <std::size_t Rows, std::size_t Cols, std::floating_point Scalar>
constexpr auto isFinite(const Matrix<Rows, Cols, Scalar>& mat) -> bool {
  for (std::size_t row = 0UZ; row < Rows; ++row) {
    for (std::size_t col = 0UZ; col < Cols; ++col) {
      if (!(absolute(mat[row, col]) <= std::numeric_limits<Scalar>::max())) {
        return false;
      }
    }
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
template <NumericType Scalar>
constexpr auto sumOfThreeProducts(Scalar x1, Scalar y1, Scalar x2, Scalar y2, Scalar x3, Scalar y3)
    -> Scalar {
  return multiplyAdd(x1, y1, multiplyAdd(x2, y2, x3 * y3));
}

//-------------------------------------------------------------------------------------------------
/// Adjugate and determinant of a small matrix: inverse = adjugate / determinant
template <std::size_t Order, NumericType Scalar>
struct Adjugate {
  Matrix<Order, Order, Scalar> adjugate;
  Scalar determinant;
};

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
struct Minors4 {
  Scalar up0;
  Scalar up1;
  Scalar up2;
  Scalar up3;
  Scalar up4;
  Scalar up5;
  Scalar lo0;
  Scalar lo1;
  Scalar lo2;
  Scalar lo3;
  Scalar lo4;
  Scalar lo5;
};

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
GRAPE_ALWAYS_INLINE constexpr auto minors4(const Matrix<4, 4, Scalar>& mat) -> Minors4<Scalar> {
  return {
    .up0 = differenceOfProducts(mat[0, 0], mat[1, 1], mat[0, 1], mat[1, 0]),
    .up1 = differenceOfProducts(mat[0, 0], mat[1, 2], mat[0, 2], mat[1, 0]),
    .up2 = differenceOfProducts(mat[0, 0], mat[1, 3], mat[0, 3], mat[1, 0]),
    .up3 = differenceOfProducts(mat[0, 1], mat[1, 2], mat[0, 2], mat[1, 1]),
    .up4 = differenceOfProducts(mat[0, 1], mat[1, 3], mat[0, 3], mat[1, 1]),
    .up5 = differenceOfProducts(mat[0, 2], mat[1, 3], mat[0, 3], mat[1, 2]),
    .lo0 = differenceOfProducts(mat[2, 0], mat[3, 1], mat[2, 1], mat[3, 0]),
    .lo1 = differenceOfProducts(mat[2, 0], mat[3, 2], mat[2, 2], mat[3, 0]),
    .lo2 = differenceOfProducts(mat[2, 0], mat[3, 3], mat[2, 3], mat[3, 0]),
    .lo3 = differenceOfProducts(mat[2, 1], mat[3, 2], mat[2, 2], mat[3, 1]),
    .lo4 = differenceOfProducts(mat[2, 1], mat[3, 3], mat[2, 3], mat[3, 1]),
    .lo5 = differenceOfProducts(mat[2, 2], mat[3, 3], mat[2, 3], mat[3, 2]),
  };
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
constexpr auto determinant4(const Minors4<Scalar>& minors) -> Scalar {
  return multiplyAdd(minors.up0, minors.lo5,
                     multiplyAdd(-minors.up1, minors.lo4,
                                 multiplyAdd(minors.up2, minors.lo3,
                                             multiplyAdd(minors.up3, minors.lo2,
                                                         multiplyAdd(-minors.up4, minors.lo1,
                                                                     minors.up5 * minors.lo0)))));
}

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
GRAPE_ALWAYS_INLINE constexpr auto adjugate(const Matrix<3, 3, Scalar>& mat)
    -> Adjugate<3, Scalar> {
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
/// Laplace expansion along rows 0-1: every cofactor is built from the same 12 2x2 minors
template <std::floating_point Scalar>
GRAPE_ALWAYS_INLINE constexpr auto adjugate(const Matrix<4, 4, Scalar>& mat)
    -> Adjugate<4, Scalar> {
  const auto minors = minors4(mat);
  const auto [up0, up1, up2, up3, up4, up5, lo0, lo1, lo2, lo3, lo4, lo5] = minors;
  const auto det = determinant4(minors);
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
/// @return Determinant of mat: closed form up to order 4, pivoted elimination above that
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
    return detail::determinant4(detail::minors4(mat));
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
      // column col below the diagonal is never read again
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
/// Gauss-Jordan elimination with partial pivoting (see inverse())
template <std::size_t Order, std::floating_point Scalar>
constexpr auto inverseGaussJordan(const Matrix<Order, Order, Scalar>& mat, Scalar mat_norm)
    -> std::optional<Matrix<Order, Order, Scalar>> {
  const auto singular_tolerance =
      static_cast<Scalar>(Order) * std::numeric_limits<Scalar>::epsilon() * mat_norm;

  // Rows are updated in full width. Columns left of col are already zero in the pivot row, but a
  // constant loop count vectorises better.
  auto lhs = mat;
  auto rhs = identity<Order, Scalar>();
  for (std::size_t col = 0UZ; col < Order; ++col) {
    const auto [pivot_row, pivot_mag] = findPivot(lhs, col);
    // NaN fails this test too
    if (!(pivot_mag > singular_tolerance)) {
      return std::nullopt;
    }
    if (pivot_row != col) {
      swapRows(lhs, col, pivot_row, col);
      swapRows(rhs, col, pivot_row);
    }
    // Local copies of the pivot row spare GCC from proving that row != col before vectorising
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
  // Pivots can pass the tolerance yet the inverse overflow (e.g. subnormal matrices)
  if (!isFinite(rhs)) {
    return std::nullopt;
  }
  return rhs;
}

}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// @return Inverse of mat, or std::nullopt if a pivot of partially pivoted elimination satisfies
/// |pivot| <= n * epsilon * ||mat||_inf. The test is relative to the scale of the whole matrix, so
/// badly scaled matrices (e.g. diag(1, 1e-300)) are rejected; rescale them before inverting.
/// Also std::nullopt if the inverse is not finite.
template <std::size_t Order, std::floating_point Scalar>
constexpr auto inverse(const Matrix<Order, Order, Scalar>& mat)
    -> std::optional<Matrix<Order, Order, Scalar>> {
  const auto mat_norm = detail::infinityNorm(mat);
  if constexpr ((Order >= 2UZ) && (Order <= 4UZ)) {
    // Closed form, used only when it agrees with elimination. Pivots grow at most as
    // |u_kk| <= 2^(k-1) * ||mat||_inf, so |det| above det_bound means no pivot is below tolerance.
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
/// std::format support. Element format specifiers apply: std::format("{:.2f}", mat) gives e.g.
/// "[[1.00, 2.00]; [3.00, 4.00]]".
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
