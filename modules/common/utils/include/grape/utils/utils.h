//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace grape::utils {

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
                                                 str.substr(0, end_pos);
}

/// Transforms C++ ABI identifiers (eg: RTTI symbols) to names used in the source code
[[nodiscard]] auto demangle(const char* mangled_name) -> std::string;

/// @return Full path of the program being executed
[[nodiscard]] auto getProgramPath() -> std::filesystem::path;

/// @return Name of the program being executed
[[nodiscard]] auto getProgramName() -> std::string;

/// @return Full path to current user's home directory
[[nodiscard]] auto getUserHomePath() -> std::filesystem::path;

/// @return Ordered list of search paths for supporting files (config, data) of an application.
/// @note Config/data file search order:
/// - User-specific application configuration: $HOME/.$APP_NAME/
/// - Host-specific application configuration: /etc/opt/$APP_NAME/
/// - Default application configuration: $APP_PATH/../share/$APP_NAME/
/// - User-specific GRAPE configuration: $HOME/.grape/
/// - Host-specific GRAPE configuration: /etc/opt/grape/
/// - Default GRAPE configuration: $GRAPE_INSTALL_PATH/share/grape/
[[nodiscard]] auto getSearchPaths() -> const std::vector<std::filesystem::path>&;

/// @return user-readable name for specified type
template <typename T>
constexpr auto getTypeName() -> std::string_view {
  constexpr std::string_view FUNCTION_NAME = __PRETTY_FUNCTION__;
  constexpr std::string_view PREFIX = "T = ";
  constexpr auto START = FUNCTION_NAME.find(PREFIX) + PREFIX.size();
  constexpr auto END = FUNCTION_NAME.find_first_of("];", START);
  return FUNCTION_NAME.substr(START, END - START);
}

}  // namespace grape::utils
