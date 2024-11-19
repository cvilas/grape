//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <format>
#include <numeric>
#include <string>

#include "grape/utils/enums.h"

namespace grape::ipc {

//=================================================================================================
struct IPAddress {
  enum class Version { IPv4, IPv6 };
  static constexpr auto MAX_SEGMENTS = 16u;

  Version version{ Version::IPv4 };
  std::array<std::uint8_t, MAX_SEGMENTS> segments;
  [[nodiscard]] static auto fromString(std::string_view ip_str) -> IPAddress;
};

//=================================================================================================
/// Defines attributes of a communication channel
struct [[nodiscard]] Locator {
  enum class Protocol { TCP, UDP };
  static constexpr auto DEFAULT_PORT = 7447;

  Protocol protocol{ Protocol::TCP };
  IPAddress address{};
  std::uint16_t port{ DEFAULT_PORT };
};

//=================================================================================================
/// Unique identifier
struct [[nodiscard]] UUID {
  static constexpr auto LENGTH = 16u;
  std::array<std::uint8_t, LENGTH> bytes;
};

//-------------------------------------------------------------------------------------------------
inline auto IPAddress::fromString(std::string_view ip_str) -> IPAddress {
  (void)ip_str;
  // resolve whether it is ipv4 or ipv6
  // if ipv6, support also compressed ipv6 notation
  return {};
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const IPAddress& addr) -> std::string {
  if (addr.version == IPAddress::Version::IPv4) {
    return std::format("{}.{}.{}.{}", addr.segments.at(0), addr.segments.at(1), addr.segments.at(2),
                       addr.segments.at(3));
  }
  std::string result;
  for (auto i = 0u; i < IPAddress::MAX_SEGMENTS; i += 2u) {
    if (i > 0) {
      result += ':';
    }
    result += std::format("{:02x}{:02x}", addr.segments.at(i), addr.segments.at(i + 1));
  }
  return result;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Locator& loc) -> std::string {
  auto proto_str = std::string(enums::enum_name(loc.protocol));
  std::ranges::transform(proto_str, proto_str.begin(), ::tolower);
  return std::format("{}/{}:{}", proto_str, toString(loc.address), loc.port);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const UUID& id) -> std::string {
  return std::accumulate(
      std::begin(id.bytes), std::end(id.bytes), std::string(),
      [](const std::string& s, std::uint8_t v) { return std::format("{:02X}", v) + s; });
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto isDefined(const UUID& id) -> bool {
  return std::ranges::any_of(id.bytes, [](const auto& b) { return b != 0; });
}

}  // namespace grape::ipc
