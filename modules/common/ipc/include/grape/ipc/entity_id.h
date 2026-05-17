//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <format>
#include <string>

namespace grape::ipc {

//=================================================================================================
// Uniquely identifies a pub/sub endpoint
struct EntityId {
  std::string host;       //!< Hostname of the remote endpoint
  std::uint64_t id{ 0 };  //!< Unique identification number
};

[[nodiscard]] constexpr auto toString(const EntityId& entity) -> std::string {
  return std::format("{}:{:#x}", entity.host.empty() ? "(unknown host)" : entity.host, entity.id);
}

}  // namespace grape::ipc
