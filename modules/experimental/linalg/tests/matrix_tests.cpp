//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cstddef>
#include <format>

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
      REQUIRE(actual[row, col] == Approx(expected[row, col]).margin(MATRIX_TEST_MARGIN));
    }
  }
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
// Every operation is usable in constant expressions. Integer matrices make the checks exact.
static_assert(identity<2, int>() == Matrix<2, 2, int>{ 1, 0, 0, 1 });
static_assert(Matrix<2, 2, int>{ 1, 2, 3, 4 } + Matrix<2, 2, int>{ 4, 3, 2, 1 } ==
              Matrix<2, 2, int>{ 5, 5, 5, 5 });
static_assert(Matrix<2, 2, int>{ 5, 5, 5, 5 } - Matrix<2, 2, int>{ 1, 2, 3, 4 } ==
              Matrix<2, 2, int>{ 4, 3, 2, 1 });
static_assert(-Matrix<2, 2, int>{ 1, 2, 3, 4 } == Matrix<2, 2, int>{ -1, -2, -3, -4 });
static_assert(Matrix<2, 2, int>{ 1, 2, 3, 4 } * 2 == Matrix<2, 2, int>{ 2, 4, 6, 8 });
static_assert(2 * Matrix<2, 2, int>{ 1, 2, 3, 4 } == Matrix<2, 2, int>{ 2, 4, 6, 8 });
static_assert(Matrix<2, 2, int>{ 1, 2, 3, 4 } * Matrix<2, 2, int>{ 5, 6, 7, 8 } ==
              Matrix<2, 2, int>{ 19, 22, 43, 50 });
static_assert(trace(Matrix<2, 2, int>{ 1, 2, 3, 4 }) == 5);
static_assert(transpose(Matrix<2, 3, int>{ 1, 2, 3, 4, 5, 6 }) ==
              Matrix<3, 2, int>{ 1, 4, 2, 5, 3, 6 });
static_assert(dot(Vector<3, int>{ 1, 2, 3 }, Vector<3, int>{ 4, 5, 6 }) == 32);
static_assert(cross(Vector<3, int>{ 1, 0, 0 }, Vector<3, int>{ 0, 1, 0 }) ==
              Vector<3, int>{ 0, 0, 1 });
static_assert(submatrix<1, 1, 2, 2>(Matrix<3, 3, int>{ 1, 2, 3, 4, 5, 6, 7, 8, 9 }) ==
              Matrix<2, 2, int>{ 5, 6, 8, 9 });

// Floating point operations are constexpr too; compare against a range to avoid exact equality
constexpr auto CONSTEXPR_DET = determinant(Matrix<2, 2>{ 1., 2., 3., 4. });
static_assert((CONSTEXPR_DET > -2.0001) && (CONSTEXPR_DET < -1.9999));

//-------------------------------------------------------------------------------------------------
TEST_CASE("Matrix initialises from comma-separated values in row-major order", "[linalg]") {
  auto mat = Matrix<2, 3>{ 1., 2., 3.,  //
                           4., 5., 6. };
  REQUIRE(mat[0, 0] == 1.);
  REQUIRE(mat[0, 2] == 3.);
  REQUIRE(mat[1, 0] == 4.);
  REQUIRE(mat[1, 2] == 6.);

  mat[1, 2] = 7.;
  REQUIRE(mat[1, 2] == 7.);

  // vectors additionally support single-index access
  const auto vec = Vector<3>{ 1., 2., 3. };
  REQUIRE(vec[1] == 2.);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Matrix element access is bounds-checked", "[linalg]") {
  auto mat = Matrix<2, 2>{};
  REQUIRE_THROWS(mat[2, 0]);
  REQUIRE_THROWS(mat[0, 2]);
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
  REQUIRE(std::format("{}", Matrix<2, 2, int>{ 1, 2, 3, 4 }) == "[[1, 2]; [3, 4]]");
  REQUIRE(std::format("{:.1f}", Vector<2>{ 0.25, -1. }) == "[[0.2]; [-1.0]]");
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
