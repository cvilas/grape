//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/utils/utils.h"

#include <array>
#include <climits>  // for PATH_MAX
#include <cstddef>
#include <memory>

#include <cxxabi.h>
#include <pwd.h>
#include <sys/types.h>  // for getpwuid
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifndef HOST_NAME_MAX
#ifdef _POSIX_HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#else
#define HOST_NAME_MAX 255
#endif
#endif

namespace {

//-------------------------------------------------------------------------------------------------
auto readProgramPath() -> std::filesystem::path {
  auto program_path = std::array<char, PATH_MAX>{};
#ifdef __APPLE__
  std::uint32_t buf_len = program_path.size();
  std::ignore = _NSGetExecutablePath(program_path.data(), &buf_len);
#elif defined(__linux__)
  std::ignore = readlink("/proc/self/exe", program_path.data(), PATH_MAX - 1);
#endif
  return { program_path.data() };
}

//-------------------------------------------------------------------------------------------------
auto readHostName() -> std::string {
  auto name = std::array<char, HOST_NAME_MAX>{};
  std::ignore = gethostname(name.data(), name.size());
  return { name.data() };
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
auto getUserHomePath() -> std::filesystem::path {
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  static const auto home_dir = std::filesystem::path(getpwuid(getuid())->pw_dir);
  return home_dir;
}

//-------------------------------------------------------------------------------------------------
auto getHostName() -> std::string {
  // cache the result for subsequent calls
  static const auto host_name = readHostName();
  return host_name;
}

//-------------------------------------------------------------------------------------------------
auto demangle(const char* mangled_name) -> std::string {
  auto status = -1;
  const auto result = std::unique_ptr<char, void (*)(void*)>{
    abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),  //
    std::free                                                      //
  };
  return { (status == 0) ? result.get() : mangled_name };
}

//-------------------------------------------------------------------------------------------------
auto getDataSearchPaths() -> const std::vector<std::filesystem::path>& {
  /// Config/data file search order:
  /// - User-specific application configuration: $HOME/.$APP_NAME/
  /// - Host-specific application configuration: /etc/opt/$APP_NAME/
  /// - Default application configuration: $APP_PATH/../share/$APP_NAME/
  /// - User-specific GRAPE configuration: $HOME/.grape/
  /// - Host-specific GRAPE configuration: /etc/opt/grape/
  /// - Default GRAPE configuration: $GRAPE_INSTALL_PATH/share/grape/
  static const auto paths = [] {
    const auto app_path = getProgramPath().parent_path();
    const auto app_name = getProgramPath().filename().string();
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
}  // namespace grape::utils
