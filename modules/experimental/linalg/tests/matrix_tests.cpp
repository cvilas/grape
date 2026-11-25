//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <format>
#include <random>
#include <stdexcept>
#include <utility>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "grape/linalg/matrix.h"

namespace {

using Catch::Approx;
using grape::linalg::cross;
using grape::linalg::determinant;
using grape::linalg::dot;
using grape::linalg::identity;
using grape::linalg::inverse;
using grape::linalg::Matrix;
using grape::linalg::setSubmatrix;
using grape::linalg::submatrix;
using grape::linalg::trace;
using grape::linalg::transpose;
using grape::linalg::Vector;

constexpr auto MATRIX_TEST_MARGIN = 1.0e-12;

template <std::size_t NumRows, std::size_t NumColumns>
void requireMatrixApprox(const Matrix<NumRows, NumColumns>& actual,
                         const Matrix<NumRows, NumColumns>& expected) {
  for (std::size_t row = 0UZ; row < NumRows; ++row) {
    for (std::size_t col = 0UZ; col < NumColumns; ++col) {
      REQUIRE(actual(row, col) == Approx(expected(row, col)).margin(MATRIX_TEST_MARGIN));
    }
  }
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
// Every operation is usable in constant expressions. Small integer values make the checks exact.
static_assert(identity<2>() == Matrix<2, 2>{ 1., 0., 0., 1. });
static_assert(Matrix<2, 2>{ 1., 2., 3., 4. } + Matrix<2, 2>{ 4., 3., 2., 1. } ==
              Matrix<2, 2>{ 5., 5., 5., 5. });
static_assert(Matrix<2, 2>{ 5., 5., 5., 5. } - Matrix<2, 2>{ 1., 2., 3., 4. } ==
              Matrix<2, 2>{ 4., 3., 2., 1. });
static_assert(-Matrix<2, 2>{ 1., 2., 3., 4. } == Matrix<2, 2>{ -1., -2., -3., -4. });
static_assert(Matrix<2, 2>{ 1., 2., 3., 4. } * 2. == Matrix<2, 2>{ 2., 4., 6., 8. });
static_assert(2. * Matrix<2, 2>{ 1., 2., 3., 4. } == Matrix<2, 2>{ 2., 4., 6., 8. });
static_assert(Matrix<2, 2>{ 1., 2., 3., 4. } * Matrix<2, 2>{ 5., 6., 7., 8. } ==
              Matrix<2, 2>{ 19., 22., 43., 50. });
static_assert(std::is_eq(trace(Matrix<2, 2>{ 1., 2., 3., 4. }) <=> 5.));
static_assert(transpose(Matrix<2, 3>{ 1., 2., 3., 4., 5., 6. }) ==
              Matrix<3, 2>{ 1., 4., 2., 5., 3., 6. });
static_assert(std::is_eq(dot(Vector<3>{ 1., 2., 3. }, Vector<3>{ 4., 5., 6. }) <=> 32.));
static_assert(cross(Vector<3>{ 1., 0., 0. }, Vector<3>{ 0., 1., 0. }) == Vector<3>{ 0., 0., 1. });
static_assert(submatrix<1, 1, 2, 2>(Matrix<3, 3>{ 1., 2., 3., 4., 5., 6., 7., 8., 9. }) ==
              Matrix<2, 2>{ 5., 6., 8., 9. });

// Floating point operations are constexpr too; compare against a range to avoid exact equality
constexpr auto CONSTEXPR_DET = determinant(Matrix<2, 2>{ 1., 2., 3., 4. });
static_assert((CONSTEXPR_DET > -2.0001) && (CONSTEXPR_DET < -1.9999));

// Elements are floating point only
template <typename Scalar>
concept ValidElement = requires { typename Matrix<1, 1, Scalar>; };
static_assert(ValidElement<float> && ValidElement<double> && ValidElement<long double>);
static_assert(not ValidElement<int> && not ValidElement<bool> && not ValidElement<char>);
static_assert(std::constructible_from<Matrix<1, 2, double>, int, int>);

// Scalar operands convert to the element type, so integer literals scale floating point matrices
static_assert(Matrix<1, 2>{ 1., 2. } * 2 == Matrix<1, 2>{ 2., 4. });
static_assert(0.5 * Matrix<1, 2, float>{ 1.F, 2.F } == Matrix<1, 2, float>{ 0.5F, 1.F });
static_assert(Matrix<1, 2>{ 3., 6. } / 3 == Matrix<1, 2>{ 1., 2. });

// Elements are exposed contiguously in row-major order
static_assert(Matrix<2, 2>{ 1., 2., 3., 4. }.elements() == std::array{ 1., 2., 3., 4. });
static_assert(std::is_eq(Matrix<2, 2>{ 1., 2., 3., 4. }(1, 0) <=> 3.));
static_assert(std::is_eq(Vector<2>{ 1., 2. }(1) <=> 2.));

// Closed-form determinants are exact for small integers. (Exact floating point comparisons in this
// file use std::is_eq(a <=> b) to state that exactness is intended.)
static_assert(std::is_eq(determinant(Matrix<3, 3>{ 2., -3., 1., 2., 0., -1., 1., 4., 5. }) <=>
                         49.));
static_assert(std::is_eq(determinant(Matrix<4, 4>{ 4., 3., 2., 1., 0., 1., -1., 2., 2., 0., 1., 3.,
                                                   1., 2., 0., -1. }) <=> 6.));

// Closed-form inverses are constexpr too
static_assert(inverse(Matrix<2, 2>{ 2., 0., 0., 4. }) == Matrix<2, 2>{ 0.5, 0., 0., 0.25 });

//-------------------------------------------------------------------------------------------------
TEST_CASE("Compile-time and runtime evaluation agree on fused arithmetic", "[linalg]") {
  // z = (1e8+1)(1e8-1) - (1e8)(1e8) = -1 exactly. Unfused, the first product rounds to 1e16 and
  // the result collapses to 0; fused (Kahan's difference of products) it is exact.
  static constexpr auto LHS = Vector<3>{ 1e8 + 1, 1e8, 3. };
  static constexpr auto RHS = Vector<3>{ 1e8, 1e8 - 1, 7. };
  const auto lhs = LHS;  // not usable in constant expressions, so cross() takes the runtime path
  const auto runtime = cross(lhs, RHS);
  REQUIRE(std::is_eq(runtime(2) <=> -1.));
#ifndef __clang__  // clang cannot yet evaluate std::fma in constant expressions
  static constexpr auto COMPILE_TIME = cross(LHS, RHS);
  static_assert(std::is_eq(COMPILE_TIME(2) <=> -1.));
  REQUIRE(runtime == COMPILE_TIME);
#endif
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Matrix initialises from comma-separated values in row-major order", "[linalg]") {
  auto mat = Matrix<2, 3>{ 1., 2., 3.,  //
                           4., 5., 6. };
  REQUIRE(mat(0, 0) == 1.);
  REQUIRE(mat(0, 2) == 3.);
  REQUIRE(mat(1, 0) == 4.);
  REQUIRE(mat(1, 2) == 6.);

  mat(1, 2) = 7.;
  REQUIRE(mat(1, 2) == 7.);

  // vectors additionally support single-index access
  const auto vec = Vector<3>{ 1., 2., 3. };
  REQUIRE(vec(1) == 2.);
}

//-------------------------------------------------------------------------------------------------
// Access to a temporary yields an rvalue, so it cannot silently dangle
static_assert(std::same_as<decltype(Matrix<2, 2>{}(0, 0)), double&&>);
static_assert(std::same_as<decltype(std::declval<const Matrix<2, 2>&>()(0, 0)), const double&>);
static_assert(std::same_as<decltype(Matrix<2, 2>{}.elements()), std::array<double, 4>&&>);

TEST_CASE("Matrix element access is bounds-checked", "[linalg]") {
  auto mat = Matrix<2, 2>{};
  const auto& const_mat = mat;
  auto vec = Vector<2>{};
  const auto& const_vec = vec;
  REQUIRE_THROWS_AS(mat(2, 0), std::out_of_range);
  REQUIRE_THROWS_AS(mat(0, 2), std::out_of_range);
  REQUIRE_THROWS_AS(const_mat(2, 0), std::out_of_range);
  REQUIRE_THROWS_AS(const_mat(0, 2), std::out_of_range);
  REQUIRE_THROWS_AS(vec(2), std::out_of_range);
  REQUIRE_THROWS_AS(const_vec(2), std::out_of_range);
  vec(1) = 3.;
  REQUIRE(const_vec(1) == 3.);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Matrix product of non-square matrices has the expected shape and values", "[linalg]") {
  const auto lhs = Matrix<2, 3>{ 1., 2., 3.,  //
                                 4., 5., 6. };
  const auto rhs = Matrix<3, 2>{ 7.,  8.,   //
                                 9.,  10.,  //
                                 11., 12. };
  requireMatrixApprox(lhs * rhs, Matrix<2, 2>{ 58., 64., 139., 154. });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Determinant matches cofactor expansion and detects singularity", "[linalg]") {
  const auto mat = Matrix<3, 3>{ 2., -3., 1.,   //
                                 2., 0.,  -1.,  //
                                 1., 4.,  5. };
  REQUIRE(determinant(mat) == Approx(49.0));

  // second row is a multiple of the first
  const auto singular = Matrix<3, 3>{ 1., 2., 3.,  //
                                      2., 4., 6.,  //
                                      7., 8., 9. };
  REQUIRE(determinant(singular) == Approx(0.0).margin(MATRIX_TEST_MARGIN));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Determinant of 4x4 matrices uses the closed form", "[linalg]") {
  // zero leading element forces a row swap (sign flip); det = -(1 * 2 * 3 * 4) by permutation
  const auto permuted = Matrix<4, 4>{ 0., 2., 0., 0.,  //
                                      1., 0., 0., 0.,  //
                                      0., 0., 3., 0.,  //
                                      0., 0., 0., 4. };
  REQUIRE(determinant(permuted) == Approx(-24.0));

  const auto general = Matrix<4, 4>{ 4., 3., 2.,  1.,  //
                                     0., 1., -1., 2.,  //
                                     2., 0., 1.,  3.,  //
                                     1., 2., 0.,  -1. };
  REQUIRE(determinant(general) == Approx(6.0));

  // fourth row is the sum of the first two
  const auto singular = Matrix<4, 4>{ 1., 2., 3., 4.,  //
                                      0., 1., 0., 1.,  //
                                      5., 6., 7., 9.,  //
                                      1., 3., 3., 5. };
  REQUIRE(determinant(singular) == Approx(0.0).margin(MATRIX_TEST_MARGIN));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Determinant of larger matrices uses pivoted elimination", "[linalg]") {
  // zero leading element forces a row swap (sign flip); det = -(1 * 2 * 3 * 4 * 5) by permutation
  const auto permuted = Matrix<5, 5>{ 0., 2., 0., 0., 0.,  //
                                      1., 0., 0., 0., 0.,  //
                                      0., 0., 3., 0., 0.,  //
                                      0., 0., 0., 4., 0.,  //
                                      0., 0., 0., 0., 5. };
  REQUIRE(determinant(permuted) == Approx(-120.0));

  const auto general = Matrix<5, 5>{ 2., 1., 0., 3., 1.,  //
                                     0., 1., 4., 1., 2.,  //
                                     3., 0., 1., 2., 0.,  //
                                     1., 2., 0., 1., 3.,  //
                                     0., 1., 2., 0., 1. };
  REQUIRE(determinant(general) == Approx(37.0));
}

//-------------------------------------------------------------------------------------------------
// Closed-form inverses (orders 2-4) must agree with Gauss-Jordan elimination, in value and in
// which matrices count as singular
template <std::size_t Order, typename Scalar>
void requireClosedFormMatchesElimination(double tolerance) {
  // NOLINTNEXTLINE(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp): repeatable inputs
  auto gen = std::mt19937{ 7U };
  auto dist = std::uniform_real_distribution<Scalar>{ Scalar{ -1 }, Scalar{ 1 } };
  for (auto trial = 0; trial < 200; ++trial) {
    auto mat = Matrix<Order, Order, Scalar>{};
    std::ranges::generate(mat.elements(), [&] { return dist(gen); });
    const auto closed = inverse(mat);
    const auto elimination =
        grape::linalg::detail::inverseGaussJordan(mat, grape::linalg::detail::infinityNorm(mat));
    REQUIRE(closed.has_value() == elimination.has_value());
    if (closed.has_value() && elimination.has_value()) {
      const auto product = mat * closed.value_or(mat);
      for (std::size_t row = 0UZ; row < Order; ++row) {
        for (std::size_t col = 0UZ; col < Order; ++col) {
          // residual scaled by the inverse's size, since random matrices can be ill-conditioned
          const auto scale =
              1.0 +
              static_cast<double>(grape::linalg::detail::infinityNorm(elimination.value_or(mat)));
          const auto expected = (row == col) ? 1.0 : 0.0;
          REQUIRE(static_cast<double>(product(row, col)) ==
                  Approx(expected).margin(tolerance * scale));
        }
      }
    }
  }
}

TEST_CASE("Closed-form inverses agree with elimination", "[linalg]") {
  requireClosedFormMatchesElimination<2, double>(1e-13);
  requireClosedFormMatchesElimination<3, double>(1e-13);
  requireClosedFormMatchesElimination<4, double>(1e-13);
  requireClosedFormMatchesElimination<2, float>(1e-4);
  requireClosedFormMatchesElimination<3, float>(1e-4);
  requireClosedFormMatchesElimination<4, float>(1e-4);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Closed-form inverse falls back to elimination for tiny but well-conditioned matrices",
          "[linalg]") {
  // the determinant (~1e-799) underflows, so the closed form cannot be used; elimination can
  const auto tiny = Matrix<4, 4>{ 4e-200, 1e-200, 0.,     0.,      //
                                  1e-200, 4e-200, 1e-200, 0.,      //
                                  0.,     1e-200, 4e-200, 1e-200,  //
                                  0.,     0.,     1e-200, 4e-200 };
  const auto inv = inverse(tiny);
  REQUIRE(inv.has_value());
  const auto product = tiny * inv.value_or(tiny);
  for (std::size_t i = 0UZ; i < 4UZ; ++i) {
    REQUIRE(product(i, i) == Approx(1.0).margin(MATRIX_TEST_MARGIN));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Inverse round-trips to identity and rejects singular matrices", "[linalg]") {
  const auto mat = Matrix<3, 3>{ 2., -3., 1.,   //
                                 2., 0.,  -1.,  //
                                 1., 4.,  5. };
  const auto inv = inverse(mat);
  REQUIRE(inv.has_value());
  const auto inv_mat = inv.value_or(Matrix<3, 3>{});  // zero fallback fails the checks below
  requireMatrixApprox(mat * inv_mat, identity<3>());
  requireMatrixApprox(inv_mat * mat, identity<3>());

  const auto singular = Matrix<3, 3>{ 1., 2., 3.,  //
                                      2., 4., 6.,  //
                                      7., 8., 9. };
  REQUIRE(!inverse(singular).has_value());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Inverse rejects matrices that are singular to working precision", "[linalg]") {
  // Rank 2 mathematically, but 0.1 etc. are inexact in binary so elimination leaves pivots of
  // rounding-noise size rather than exact zeros. An absolute zero-pivot test would accept these
  // and return an "inverse" with entries around 1e15.
  const auto decimal = Matrix<3, 3>{ 0.1, 0.2, 0.3,  //
                                     0.4, 0.5, 0.6,  //
                                     0.7, 0.8, 0.9 };
  REQUIRE(!inverse(decimal).has_value());

  // third row is the sum of the first two
  const auto dependent = Matrix<3, 3>{ 0.3, 0.6, 0.9,  //
                                       0.1, 0.7, 0.2,  //
                                       0.4, 1.3, 1.1 };
  REQUIRE(!inverse(dependent).has_value());

  // well-conditioned matrices at any overall scale are still accepted
  const auto tiny = Matrix<2, 2>{ 2e-200, 1e-200,  //
                                  1e-200, 3e-200 };
  REQUIRE(inverse(tiny).has_value());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Inverse rejects matrices whose inverse is not representable", "[linalg]") {
  // Pivots pass the relative tolerance, but 1/pivot overflows
  constexpr auto SUBNORMAL = 4e-309;
  const auto subnormal = Matrix<2, 2>{ SUBNORMAL, SUBNORMAL,  //
                                       SUBNORMAL, -SUBNORMAL };
  REQUIRE(!inverse(subnormal).has_value());

  const auto subnormal3 = Matrix<3, 3>{ SUBNORMAL, 0.,        0.,  //
                                        0.,        SUBNORMAL, 0.,  //
                                        0.,        0.,        SUBNORMAL };
  REQUIRE(!inverse(subnormal3).has_value());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Inverse pivots rows when a leading diagonal element is zero", "[linalg]") {
  const auto mat = Matrix<2, 2>{ 0., 1.,  //
                                 1., 0. };
  const auto inv = inverse(mat);
  REQUIRE(inv.has_value());
  requireMatrixApprox(mat * inv.value_or(Matrix<2, 2>{}), identity<2>());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Linear system y = Ax + b solves for x", "[linalg]") {
  const auto a_mat = Matrix<3, 3>{ 2., 0., 0.,  //
                                   0., 4., 0.,  //
                                   0., 0., 8. };
  const auto b_vec = Vector<3>{ 1., 1., 1. };
  const auto x_expected = Vector<3>{ 1., 2., 3. };
  const auto y_vec = (a_mat * x_expected) + b_vec;

  const auto a_inv = inverse(a_mat);
  REQUIRE(a_inv.has_value());
  requireMatrixApprox(a_inv.value_or(Matrix<3, 3>{}) * (y_vec - b_vec), x_expected);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Cross product is anti-commutative and orthogonal to its operands", "[linalg]") {
  const auto va = Vector<3>{ 1., 2., 3. };
  const auto vb = Vector<3>{ -5., 7., 2. };
  const auto prod = cross(va, vb);

  requireMatrixApprox(prod, -cross(vb, va));
  REQUIRE(dot(prod, va) == Approx(0.0).margin(MATRIX_TEST_MARGIN));
  REQUIRE(dot(prod, vb) == Approx(0.0).margin(MATRIX_TEST_MARGIN));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Vector norm is the euclidean length", "[linalg]") {
  REQUIRE(norm(Vector<3>{ 3., 4., 0. }) == Approx(5.0));
  REQUIRE(norm(Vector<2>{ 0., 0. }) == Approx(0.0).margin(MATRIX_TEST_MARGIN));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Submatrix blocks can be read and written", "[linalg]") {
  auto mat = Matrix<3, 3>{ 1., 2., 3.,  //
                           4., 5., 6.,  //
                           7., 8., 9. };
  requireMatrixApprox(submatrix<0, 1, 2, 2>(mat), Matrix<2, 2>{ 2., 3., 5., 6. });

  setSubmatrix<1, 1>(mat, Matrix<2, 2>{ 0., 0., 0., 0. });
  requireMatrixApprox(mat, Matrix<3, 3>{ 1., 2., 3.,  //
                                         4., 0., 0.,  //
                                         7., 0., 0. });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Matrix formats with std::format, honouring element format specifiers", "[linalg]") {
  REQUIRE(std::format("{}", Matrix<2, 2>{ 1., 2., 3., 4. }) == "[[1, 2]; [3, 4]]");
  REQUIRE(std::format("{:.1f}", Vector<2>{ 0.25, -1. }) == "[[0.2]; [-1.0]]");
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
