//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/utils/file_system.h"

//=================================================================================================
auto main() -> int {
  try {
    const auto path = grape::utils::getProgramPath();
    const auto host_name = grape::utils::getHostName();
    const auto home_dir = grape::utils::getUserHomePath();
    std::println("Host name: {}", host_name);
    std::println("User's home: {}", home_dir.string());
    std::println("Program name: {}", grape::utils::getProgramName());
    std::println("Program path: {}", path.parent_path().string());
    std::println("Data search directories for this application:");
    for (const auto& dir : grape::utils::getSearchPaths()) {
      std::println("\t{}", dir.string());
    }
    return EXIT_SUCCESS;
  } catch (...) {
    std::ignore = std::fputs("Exception occurred", stderr);
    return EXIT_FAILURE;
  }
}
