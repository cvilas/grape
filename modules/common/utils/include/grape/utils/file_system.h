//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace grape::utils {

/// Returns 'system' name by searching for it in the following order:
/// - Environment variable `SYSTEM_NAME` (akin to HOSTNAME shell variable)
/// - A `system_name` file in the standard search paths (see getSearchPaths()). This is analogous
///   in function and file format to `/etc/hostname` on Unix
/// - Host name (fallback, if above fails)
[[nodiscard]] auto getSystemName() -> std::string;

/// @return host name
[[nodiscard]] auto getHostName() -> std::string;

/// @return Full path of the program being executed
[[nodiscard]] auto getProgramPath() -> std::filesystem::path;

/// @return Name of the program being executed
[[nodiscard]] auto getProgramName() -> std::string;

/// @return Full path to current user's home directory
[[nodiscard]] auto getUserHomePath() -> std::filesystem::path;

/// @return Ordered list of 'standard' search paths for supporting data files (config, models, etc)
/// @note Config/data file search order:
/// - User-specific application configuration: $HOME/.$APP_NAME/
/// - Host-specific application configuration: /etc/opt/$APP_NAME/
/// - Default application configuration: $APP_PATH/../share/$APP_NAME/
/// - User-specific GRAPE configuration: $HOME/.grape/
/// - Host-specific GRAPE configuration: /etc/opt/grape/
/// - Default GRAPE configuration: $GRAPE_INSTALL_PATH/share/grape/
[[nodiscard]] auto getSearchPaths() -> const std::vector<std::filesystem::path>&;

/// Search in standard locations to resolve absolute path to a file.
/// @param file_name Name of the file to search, which could be absolute or relative path
/// @return Absolute path to the file, if found
[[nodiscard]] auto resolveFilePath(const std::filesystem::path& file_name)
    -> std::optional<std::filesystem::path>;

}  // namespace grape::utils
