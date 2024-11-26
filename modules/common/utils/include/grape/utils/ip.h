//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace grape::utils {

/// Holds an IP Address and associated methods
struct [[nodiscard]] IPAddress {
  enum class Version { IPv4, IPv6 };
  static constexpr auto MAX_SEGMENTS = 16u;

  Version version{ Version::IPv4 };
  std::array<std::uint8_t, MAX_SEGMENTS> bytes{};

  [[nodiscard]] static auto fromString(const std::string& ip_str) -> std::optional<IPAddress>;
};

/// @return string representation of an IP address
[[nodiscard]] auto toString(const IPAddress& addr) -> std::string;

/// @return host name
[[nodiscard]] auto getHostName() -> std::string;

}  // namespace grape::utils
