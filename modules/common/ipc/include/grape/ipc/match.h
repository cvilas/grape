//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <format>
#include <functional>
#include <string>

#include "grape/utils/enums.h"

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

//=================================================================================================
/// Match event between a pub<->sub pair
struct Match {
  enum class Status : std::uint8_t {
    Unmatched,  //!< a previously matched remote endpoint is no longer available
    Matched     //!< matched a new remote endpoint
  };
  EntityId remote_entity;  //!< Remote endpoint identifier
  Status status{};
};

[[nodiscard]] constexpr auto toString(Match::Status status) -> std::string_view {
  return grape::enums::name(status);
}

/// Function signature for callback on match event
using MatchCallback = std::function<void(const Match&)>;
}  // namespace grape::ipc
