//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <functional>

namespace grape::ipc {

/// Represents a match event between a subscriber and a publisher
struct Match {
  enum class Status : std::uint8_t {
    Undefined,  //!< unknown status
    Unmatched,  //!< a previously matched remote endpoint is no longer available
    Matched     //!< matched a new remote endpoint
  };

  Status status{};
};

/// Function signature for callback on match event
using MatchCallback = std::function<void(const Match&)>;
}  // namespace grape::ipc
