//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>
#include <span>

#include "grape/ipc/match.h"
#include "grape/ipc/topic.h"

namespace grape::ipc {

//=================================================================================================
/// Publishers post topic data. Created by Session. See also Topic.
class Publisher {
public:
  /// Creates a publisher
  /// @param topic topic attributes
  /// @param match_cb Match callback, triggered on matched/unmatched with a remote subscriber
  explicit Publisher(const Topic& topic, MatchCallback&& match_cb = nullptr);

  /// Publish data on topic specified at creation by Session
  void publish(std::span<const std::byte> bytes) const;

  ~Publisher();
  Publisher(Publisher&&) noexcept;
  Publisher(const Publisher&) = delete;
  auto operator=(const Publisher&) = delete;
  auto operator=(Publisher&&) noexcept = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::ipc
