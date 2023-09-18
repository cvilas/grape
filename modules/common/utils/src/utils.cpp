//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "grape/utils/utils.h"

#include <algorithm>

#include "version_impl.h"

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto trim(const std::string& str) -> std::string {
  std::string result = str;

  // Trim leading whitespace
  result.erase(result.begin(), std::find_if(result.begin(), result.end(),
                                            [](int ch) { return 0 == std::isspace(ch); }));

  // Trim trailing whitespace
  result.erase(
      std::find_if(result.rbegin(), result.rend(), [](int ch) { return 0 == std::isspace(ch); })
          .base(),
      result.end());

  return result;
}

//-------------------------------------------------------------------------------------------------
auto getSourcePath() -> std::filesystem::path {
  return { SOURCE_PATH };
}

}  // namespace grape::utils