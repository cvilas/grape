//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

//=================================================================================================
// Note: Adds helper functions to support std::format for container-like types when using GCC.
// (Clang/libc++ already has this support).
// Remove this file when GCC adds support for formatting ranges
//=================================================================================================

#include <concepts>
#include <format>
#include <version>  // For feature test macros

//=================================================================================================
// Concept to check if a type is a container with begin() and end() methods
template <typename T>
concept ContainerLike = requires(T& obj) {
  { std::begin(obj) } -> std::input_iterator;
  { std::end(obj) } -> std::sentinel_for<decltype(std::begin(obj))>;
};

//=================================================================================================
// Specialised formatter template for container-like types
template <ContainerLike Container>
struct std::formatter<Container> {  // NOLINT(cert-dcl58-cpp)
  static constexpr auto DELIM_OPEN = '[';
  static constexpr auto DELIM_CLOSE = ']';
  static constexpr auto SEPARATOR = ',';

  constexpr auto parse(std::format_parse_context& ctx) {
    const auto* it = ctx.begin();
    const auto* const end = ctx.end();
    while (it != end && *it != '}') {
      ++it;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const Container& container, FormatContext& ctx) const {
    auto out = ctx.out();
    *out++ = DELIM_OPEN;
    auto it = std::begin(container);
    const auto end = std::end(container);
    if (it != end) {
      out = std::format_to(out, "{}", *it);
      ++it;
      for (; it != end; ++it) {
        out = std::format_to(out, "{} {}", SEPARATOR, *it);
      }
    }
    *out++ = DELIM_CLOSE;
    return out;
  }
};
