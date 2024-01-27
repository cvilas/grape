//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/utils/version.h"

#include "version_impl.h"

namespace grape::utils {
auto getBuildInfo() -> BuildInfo {
  return { REPO_BRANCH, BUILD_PROFILE, REPO_HASH };
}

auto getVersion() -> Version {
  return { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH };
}

}  // namespace grape::utils
