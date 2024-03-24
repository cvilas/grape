//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <typeinfo>

#include <cxxabi.h>

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

/*
/// Return user-readable name for specified type
template <typename T>
auto getTypeName() -> std::string {
  // From https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
  const auto* const mangled_name = typeid(T).name();
  int status{ 0 };
  std::unique_ptr<char, void (*)(void*)> res{
    abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), std::free
  };
  return (status == 0) ? res.get() : mangled_name;
}
*/

/// Return user-readable name for specified type
template <class T>
constexpr auto getTypeName() -> std::string_view {
  // From https://stackoverflow.com/a/35943472/9929294
  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  // clang-format off
  char const* p = __PRETTY_FUNCTION__;
  while (*p++ != '='){}
  for (; *p == ' '; ++p){}
  char const* p2 = p;
  int count = 1;
  for (;; ++p2) {
    switch (*p2) {
      case '[': ++count; break;
      case ']': --count; if (!count) { return { p, static_cast<std::size_t>(p2 - p) }; } break;
      default: break;
    }
  }
  // clang-format on
  // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  return {};
}

}  // namespace grape::utils
