//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstdio>
#include <cstdlib>
#include <print>
#include <string>
#include <tuple>

#include "grape/utils/file_system.h"

//=================================================================================================
auto main() -> int {
  try {
    std::println("{}", grape::utils::getSystemName());
  } catch (...) {
    std::ignore = std::fputs("Exception occurred", stderr);
    return EXIT_FAILURE;
  }
}
