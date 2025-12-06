//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstdlib>

#include "grape/linalg/matrix.h"

//=================================================================================================
auto main() -> int {
  [[maybe_unused]] auto a_mat = grape::linalg::Matrix<3, 3>{};
  /*
  auto b = grape::linalg::Vector<3>{};
  auto y = grape::linalg::Vector<3>{};

  // solve x in y = Ax + b
  const auto x = inverse(A) * (y - b);
  std::println("{}", x);
  */
  return EXIT_SUCCESS;
}
