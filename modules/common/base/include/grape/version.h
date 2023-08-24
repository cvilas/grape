//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include <cinttypes>
#include <compare>
#include <format>
#include <string_view>

namespace grape {

struct [[nodiscard]] Version {
  uint8_t major{};
  uint8_t minor{};
  uint16_t patch{};
  constexpr auto operator<=>(const Version&) const = default;
};

struct [[nodiscard]] BuildInfo {
  std::string_view branch;
  std::string_view profile;
  std::string_view hash;
};

auto getVersion() -> Version;

auto getBuildInfo() -> BuildInfo;

}  // namespace grape

template <>
struct std::formatter<grape::Version> : std::formatter<std::string> {
  static auto format(const grape::Version& v, std::format_context& ctx) {
    return std::format_to(ctx.out(), "{:d}.{:d}.{:d}", v.major, v.minor, v.patch);
  }
};

template <>
struct std::formatter<grape::BuildInfo> : std::formatter<std::string> {
  static auto format(const grape::BuildInfo& bi, std::format_context& ctx) {
    return std::format_to(ctx.out(), "'{}' branch, '{}' profile, '{}' hash", bi.branch, bi.profile,
                          bi.hash);
  }
};
