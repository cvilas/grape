//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <sstream>
#include <string>

namespace grape::utils {

/// Defines a concept to check if a type can be converted from a string
template <typename T>
concept istringstreamable =
    requires(std::string str, T value) { std::istringstream{ str } >> value; };

/// Truncates a string by returning the segment between start and end token strings, including
/// the start token but excluding the end token. For example, the following code will return
/// 'to/some/file'
/// @code
/// constexpr auto str = "/path/to/some/file.txt";
/// constexpr auto start_token = "to";
/// constexpr auto end_token = ".txt";
/// constexpr auto truncated = truncate(str, start_token, end_token);
/// std::cout << truncated << '\n';
/// @endcode
constexpr auto truncate(std::string_view str, std::string_view start_token,
                        std::string_view end_token = std::string_view("")) -> std::string_view {
  const auto start_pos = str.find(start_token);
  const auto end_pos = end_token.empty() ? std::string_view::npos : str.find(end_token);
  return (start_pos != std::string_view::npos) ? str.substr(start_pos, end_pos - start_pos) :
                                                 str.substr(0, end_pos - start_pos);
}

}  // namespace grape::utils
