//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/utility.h"

#include "version_impl.h"

namespace grape {
auto getSourcePath() -> std::filesystem::path {
  return { GRAPE_SOURCE_PATH };
}

}  // namespace grape