//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/version.h"

#include "version_impl.h"

namespace grape {
auto getBuildInfo() -> BuildInfo {
  return { REPO_BRANCH, BUILD_PROFILE, REPO_HASH };
}

auto getVersion() -> Version {
  return { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH };
}

}  // namespace grape
