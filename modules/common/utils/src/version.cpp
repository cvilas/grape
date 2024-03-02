//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/utils/version.h"

#include "version_impl.h"

namespace grape::utils {
auto getBuildInfo() -> BuildInfo {
  return { .branch = REPO_BRANCH, .profile = BUILD_PROFILE, .hash = REPO_HASH };
}

auto getVersion() -> Version {
  return { .major = VERSION_MAJOR, .minor = VERSION_MINOR, .patch = VERSION_PATCH };
}

}  // namespace grape::utils
