//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstdio>
#include <cstdlib>
#include <print>
#include <string_view>
#include <tuple>

#include "grape/utils/version.h"

//=================================================================================================
auto main() -> int {
  try {
    const auto vn = grape::utils::getVersion();
    const auto bi = grape::utils::getBuildInfo();
    std::println("Version    : {:d}.{:d}.{:d}", vn.major, vn.minor, vn.patch);
    std::println("Build Info : '{}' branch, '{}' profile, '{}' hash", bi.branch, bi.profile,
                 bi.hash);
    return EXIT_SUCCESS;
  } catch (...) {
    std::ignore = std::fputs("Exception occurred", stderr);
    return EXIT_FAILURE;
  }
}
