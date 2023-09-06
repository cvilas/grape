//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <format>
#include <iostream>

#include "grape/version.h"

//=================================================================================================
auto main() -> int {
  const auto vn = grape::getVersion();
  const auto bi = grape::getBuildInfo();
  std::cout << std::format("Version    : {:d}.{:d}.{:d}\n", vn.major, vn.minor, vn.patch);
  std::cout << std::format("Build Info : '{}' branch, '{}' profile, '{}' hash\n", bi.branch,
                           bi.profile, bi.hash);
  return EXIT_SUCCESS;
}
