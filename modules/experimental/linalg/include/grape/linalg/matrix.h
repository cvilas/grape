//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <type_traits>

template <typename Scaler, std::size_t NumRows, std::size_t NumColumns>
  requires std::is_arithmetic_v<Scaler>
class Matrix {
public:
  constexpr Matrix(const Scaler& initial_value = {}) : storage_(initial_value) {
  }
  constexpr auto operator[](std::size_t row, std::size_t col) -> T& {
    return storage_[row * NumColumns + col];
  }
  constexpr auto operator[](std::size_t row, std::size_t col) const -> const T& {
    return storage_[row * NumColumns + col];
  }

  // todo - make this clear
  template <std::size_t NumRows2, std::size_t NumColumns2>
    requires std::is_same_v<NumRows2, NumColumns>
  constexpr auto operator*(const Matrix<Scaler, NumRows2, NumColumns2>& m2) {
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
  std::array<NumRows * NumColumns, Scaler> storage_;
};