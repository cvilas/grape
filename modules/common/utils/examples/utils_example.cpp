//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/utils/utils.h"

//=================================================================================================
auto main() -> int {
  try {
    const auto path = grape::utils::getProgramPath();
    const auto host_name = grape::utils::getHostName();
    std::println("Host name: {}", host_name);
    std::println("Program name: {}", path.filename().string());
    std::println("Program path: {}", path.parent_path().string());
    return EXIT_SUCCESS;
  } catch (...) {
    std::ignore = std::fputs("Exception occurred", stderr);
    return EXIT_FAILURE;
  }
}