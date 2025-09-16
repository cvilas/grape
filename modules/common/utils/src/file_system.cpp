//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/utils/file_system.h"

#include <array>
#include <climits>  // for PATH_MAX

#include <pwd.h>
#include <sys/types.h>  // for getpwuid
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace {

//-------------------------------------------------------------------------------------------------
auto readProgramPath() -> std::filesystem::path {
  auto program_path = std::array<char, PATH_MAX>{};
#ifdef __APPLE__
  std::uint32_t buf_len = program_path.size();
  std::ignore = _NSGetExecutablePath(program_path.data(), &buf_len);
#elifdef __linux__
  std::ignore = readlink("/proc/self/exe", program_path.data(), PATH_MAX - 1);
#endif
  return { program_path.data() };
}

}  // namespace

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto getProgramPath() -> std::filesystem::path {
  // cache the result for subsequent calls
  static const auto program_path = readProgramPath();
  return program_path;
}

//-------------------------------------------------------------------------------------------------
auto getProgramName() -> std::string {
  return getProgramPath().filename().string();
}

//-------------------------------------------------------------------------------------------------
auto getUserHomePath() -> std::filesystem::path {
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  static const auto home_dir = std::filesystem::path(getpwuid(getuid())->pw_dir);
  return home_dir;
}

//-------------------------------------------------------------------------------------------------
auto getSearchPaths() -> const std::vector<std::filesystem::path>& {
  static const auto paths = [] -> std::vector<std::filesystem::path> {
    const auto app_path = getProgramPath().parent_path();
    const auto app_name = getProgramName();
    const auto home_path = getUserHomePath();
    return std::vector<std::filesystem::path>{
      home_path / ("." + app_name),         //
      ("/etc/opt/" + app_name),             //
      app_path / ("../share/" + app_name),  //
      home_path / (".grape"),               //
      ("/etc/opt/grape"),                   //
      app_path / ("../share/grape")         //
    };
  }();
  return paths;
}

//-------------------------------------------------------------------------------------------------
auto resolveFilePath(const std::filesystem::path& file_name)
    -> std::optional<std::filesystem::path> {
  // try the user-specified path first...
  if (std::filesystem::exists(file_name) && std::filesystem::is_regular_file(file_name)) {
    return std::filesystem::absolute(file_name);
  }
  // otherwise search for it...
  const auto& search_dirs = grape::utils::getSearchPaths();
  for (const auto& dir : search_dirs) {
    auto full_path = dir / file_name;
    if (std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path)) {
      return full_path;
    }
  }
  return std::nullopt;
}

}  // namespace grape::utils
