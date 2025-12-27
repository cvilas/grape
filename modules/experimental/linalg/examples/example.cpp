//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstdlib>

#include "grape/linalg/linalg.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  Matrix<double, 3, 3> A;
  Vector<double, 3> b;
  Vector<double, 3> y;

  // solve x in y = Ax + b
  const auto x = inverse(A) * (y - b);
  std::println("{}", x);
  (void)argc;
  (void)argv;
  return EXIT_SUCCESS;
}
