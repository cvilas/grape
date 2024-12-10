//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <format>
#include <optional>
#include <string>

#include "grape/utils/enums.h"
#include "grape/utils/ip.h"

namespace grape::ipc {

//=================================================================================================
/// Defines attributes of a network connection endpoint
struct [[nodiscard]] Locator {
  enum class Protocol : std::uint8_t { TCP, UDP };
  static constexpr auto DEFAULT_PORT = 7447;

  Protocol protocol{ Protocol::TCP };
  utils::IPAddress address{};
  std::uint16_t port{ DEFAULT_PORT };

  /// Construct from string representation. String must be formatted as `protocol/address:port`.
  /// Examples:
  /// - ipv6 format: "tcp/[fe80::2145:12c5:9fc3:3c71]:7447"
  /// - ipv4 format: "tcp/192.168.0.2:7447"
  /// @param loc_str String specification of locator
  /// @return Locator on success, nothing on parsing error
  [[nodiscard]] static auto fromString(const std::string& loc_str) -> std::optional<Locator>;
};

//=================================================================================================
/// Unique identifier
struct [[nodiscard]] UUID {
  static constexpr auto LENGTH = 16U;
  std::array<std::uint8_t, LENGTH> bytes;
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Locator& loc) -> std::string {
  // ipv6 format: tcp/[fe80::2145:12c5:9fc3:3c71]:7447
  // ipv4 format: tcp/192.168.0.2:7447
  auto proto_str = std::string(enums::name(loc.protocol));
  std::ranges::transform(proto_str, proto_str.begin(), ::tolower);
  auto addr_str = toString(loc.address);
  if (loc.address.version == utils::IPAddress::Version::IPv6) {
    addr_str.insert(0, 1, '[');
    addr_str.push_back(']');
  }
  return std::format("{}/{}:{}", proto_str, addr_str, loc.port);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const UUID& id) -> std::string {
  return std::ranges::fold_left(
      id.bytes, std::string(),
      [](const std::string& str, std::uint8_t val) { return std::format("{:02X}", val) + str; });
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto isDefined(const UUID& id) -> bool {
  return std::ranges::any_of(id.bytes, [](const auto& ch) { return ch != 0; });
}

}  // namespace grape::ipc
