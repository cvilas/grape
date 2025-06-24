//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "grape/utils/enums.h"

namespace grape::ipc {

/// Represents a match event between a subscriber and a publisher
struct Match {
  enum class Status : std::uint8_t {
    Undefined,  //!< unknown status
    Unmatched,  //!< a previously matched remote endpoint is no longer available
    Matched     //!< matched a new remote endpoint
  };
  std::uint64_t id{ 0 };  //!< unique identifier of the remote endpoint
  std::string host;       //!< Hotname at the remote endpoint
  Status status{};
};

[[nodiscard]] constexpr auto toString(Match::Status status) -> std::string_view {
  return grape::enums::name(status);
}

/// Function signature for callback on match event
using MatchCallback = std::function<void(const Match&)>;
}  // namespace grape::ipc
