//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <format>
#include <iostream>

#include "grape/version.h"

//=================================================================================================
auto main() -> int {
  std::cout << std::format("Version    : {}\n", grape::getVersion());
  std::cout << std::format("Build Info : {}\n", grape::getBuildInfo());
  return EXIT_SUCCESS;
}
