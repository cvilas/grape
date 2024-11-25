//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#include "grape/ipc/common.h"

#include <limits>

#include "grape/utils/enums.h"

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
auto Locator::fromString(const std::string& loc_str) -> std::optional<Locator> {
  // Find the protocol separator
  auto protocol_sep_pos = loc_str.find('/');
  if (protocol_sep_pos == std::string::npos) {
    return std::nullopt;
  }

  // Find port separator
  auto port_sep_pos = loc_str.rfind(':');
  if (port_sep_pos == std::string::npos) {
    return std::nullopt;
  }

  // Extract protocol
  auto protocol_str = loc_str.substr(0, protocol_sep_pos);
  std::ranges::transform(protocol_str, protocol_str.begin(), ::toupper);
  const auto protocol = enums::enum_cast<Locator::Protocol>(protocol_str);
  if (not protocol) {
    return std::nullopt;
  }

  // Extract address. Check and remove parenthesis if specified
  auto addr_str = loc_str.substr(protocol_sep_pos + 1, port_sep_pos - protocol_sep_pos - 1);
  if (addr_str.front() == '[' && addr_str.back() == ']') {
    addr_str = addr_str.substr(1, addr_str.length() - 2);
  }
  const auto addr = utils::IPAddress::fromString(addr_str);
  if (not addr) {
    return std::nullopt;
  }

  // Extract port
  int port{};
  try {
    const auto port_str = loc_str.substr(port_sep_pos + 1);
    port = std::stoi(port_str);
    if ((port < std::numeric_limits<std::uint16_t>::min()) or
        (port > std::numeric_limits<std::uint16_t>::max())) {
      return std::nullopt;
    }
  } catch (...) {
    return std::nullopt;
  }

  // Construct and return Locator
  return Locator{ .protocol = *protocol,
                  .address = *addr,
                  .port = static_cast<std::uint16_t>(port) };
}

}  // namespace grape::ipc
