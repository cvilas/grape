//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <iostream>

#include "grape/exception.h"

//=================================================================================================
/// Demonstrates how exceptions should be thrown and caught
auto main() -> int {
  try {
    throw grape::Exception("Runtime exception");
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << "\n";
  }
  return EXIT_SUCCESS;
}
