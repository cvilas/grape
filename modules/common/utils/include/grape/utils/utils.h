//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <sstream>
#include <string>

namespace grape::utils {

/// Defines a concept to check if a type can be converted from a string
template <typename T>
concept istringstreamable =
    requires(std::string str, T value) { std::istringstream{ str } >> value; };

/// Trims whitespace at the beginning and end of a string
auto trim(const std::string& str) -> std::string;

/// @return Location on disk where the source code was built from.
auto getSourcePath() -> std::filesystem::path;

}  // namespace grape::utils