//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include <cinttypes>
#include <compare>
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
