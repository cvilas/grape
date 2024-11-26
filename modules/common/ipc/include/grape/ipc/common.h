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
#include "grape/utils/ip.h"

namespace grape::ipc {

//=================================================================================================
/// Defines attributes of a communication channel
struct [[nodiscard]] Locator {
  enum class Protocol { TCP, UDP };
  static constexpr auto DEFAULT_PORT = 7447;

  Protocol protocol{ Protocol::TCP };
  utils::IPAddress address{};
  std::uint16_t port{ DEFAULT_PORT };
};

//=================================================================================================
/// Unique identifier
struct [[nodiscard]] UUID {
  static constexpr auto LENGTH = 16u;
  std::array<std::uint8_t, LENGTH> bytes;
};

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
