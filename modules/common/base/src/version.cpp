//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/version.h"

#include "version_impl.h"

namespace grape {
auto getBuildInfo() -> BuildInfo {
  return { GRAPE_REPO_BRANCH, GRAPE_BUILD_PROFILE, GRAPE_REPO_HASH };
}

auto getVersion() -> Version {
  return { GRAPE_VERSION_MAJOR, GRAPE_VERSION_MINOR, GRAPE_VERSION_PATCH };
}

}  // namespace grape
