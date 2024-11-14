//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/utils/utils.h"

#include <array>
#include <climits>  // for PATH_MAX
#include <cstddef>
#include <memory>

#include <cxxabi.h>
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
  return (status == 0) ? result.get() : mangled_name;
}

}  // namespace grape::utils
