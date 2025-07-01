//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>
#include <span>

#include "grape/ipc/match.h"

namespace grape::ipc {

//=================================================================================================
/// Publishers post topic data. Created by Session.
class Publisher {
public:
  /// Creates a publisher
  /// @param topic Topic on which to publish data
  /// @param match_cb Match callback, triggered on matched/unmatched with a remote subscriber
  explicit Publisher(const std::string& topic, MatchCallback&& match_cb = nullptr);

  /// Publish data on topic specified at creation by Session
  void publish(std::span<const std::byte> bytes) const;

  /// @return Unique identifier for this endpoint on the network
  [[nodiscard]] auto id() const -> std::uint64_t;

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
