//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <expected>
#include <memory>
#include <span>

#include "grape/ipc/error.h"
#include "grape/ipc/match.h"

namespace grape::ipc {

//=================================================================================================
/// Publishers post data on a topic.
///
class RawPublisher {
public:
  /// Creates a publisher
  /// @param topic Topic on which to publish data
  /// @param match_cb Match callback, triggered on matched/unmatched with a remote subscriber
  explicit RawPublisher(const std::string& topic, MatchCallback&& match_cb = nullptr);

  /// Publish data on topic specified at construction
  /// @return nothing on success, error on failure
  [[nodiscard]] auto publish(std::span<const std::byte> bytes) const -> std::expected<void, Error>;

  /// @return The number of subscribers currently matched to this publisher
  [[nodiscard]] auto subscriberCount() const -> std::size_t;

  /// @return Unique identifier for this endpoint on the network
  [[nodiscard]] auto id() const -> std::uint64_t;

  virtual ~RawPublisher();
  RawPublisher(RawPublisher&&) noexcept;
  RawPublisher(const RawPublisher&) = delete;
  auto operator=(const RawPublisher&) = delete;
  auto operator=(RawPublisher&&) noexcept = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::ipc
