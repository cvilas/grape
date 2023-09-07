//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <sstream>
#include <string>

namespace grape::utils {

/// Defines a concept to check if a type can be converted from a string
template <typename T>
concept ConvertibleFromString =
    requires(std::string str, T value) { std::istringstream{ str } >> value; };

/// Trims whitespace at the beginning and end of a string
auto trim(const std::string& str) -> std::string;

}  // namespace grape::utils