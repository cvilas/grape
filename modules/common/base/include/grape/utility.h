//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include <filesystem>

namespace grape {
/// \return Location on disk where the source code was built from.
auto getSourcePath() -> std::filesystem::path;
}  // namespace grape