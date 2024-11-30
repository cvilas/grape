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

  /// Construct from string representation. String must be formatted as follows:
  /// - ipv6 format example: "fe80::2145:12c5:9fc3:3c71"
  /// - ipv4 format example: "192.168.0.2"
  /// @param ip_str String specification of IP address
  /// @return IP address on success, nothing on parsing error
  [[nodiscard]] static auto fromString(const std::string& ip_str) -> std::optional<IPAddress>;
};

/// @return string representation of an IP address
[[nodiscard]] auto toString(const IPAddress& addr) -> std::string;

/// @return host name
[[nodiscard]] auto getHostName() -> std::string;

}  // namespace grape::utils
