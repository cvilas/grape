//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>

#include "grape/ipc/entity_id.h"
#include "grape/ipc/topic.h"
#include "grape/utils/enums.h"

namespace grape::ipc {

//=================================================================================================
/// Match event between a pub<->sub pair
struct Match {
  enum class Status : std::uint8_t {
    Unmatched,  //!< a previously matched remote endpoint is no longer available
    Matched,    //!< matched a new remote endpoint
  };
  EntityId remote_entity;  //!< Remote endpoint identifier
  Status status{};
  Topic topic{};
};

[[nodiscard]] constexpr auto toString(Match::Status status) -> std::string_view {
  return grape::enums::name(status);
}

/// Function signature for callback on match event
using MatchCallback = std::function<void(const Match&)>;

}  // namespace grape::ipc
