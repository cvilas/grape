//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <span>

#include "grape/ipc/match.h"

namespace grape::ipc {

//=================================================================================================
/// Meta information about data contained in a sample
struct SampleInfo {
  std::chrono::system_clock::time_point publish_time;
  EntityId publisher;
};

//=================================================================================================
/// Defines data sample received by the subscriber, with related meta-information
struct Sample {
  std::span<const std::byte> data;
  SampleInfo info;
};

//=================================================================================================
/// Subscribers receive topic data.
class RawSubscriber {
public:
  /// Function signature for callback on received data
  using DataCallback = std::function<void(const Sample&)>;

  /// creates a subscriber
  /// @param topic Topic on which to listen to for data from matched publishers
  /// @param data_cb Data processing callback, triggered on every newly received data sample
  /// @param match_cb Match callback, triggered when matched/unmatched with a remote publisher
  RawSubscriber(const std::string& topic, DataCallback&& data_cb,
                MatchCallback&& match_cb = nullptr);

  /// @return The number of publishers currently matched to this subscriber
  [[nodiscard]] auto publisherCount() const -> std::size_t;

  /// @return Unique identifier for this endpoint on the network
  [[nodiscard]] auto id() const -> std::uint64_t;

  virtual ~RawSubscriber();
  RawSubscriber(RawSubscriber&&) noexcept;
  RawSubscriber(const RawSubscriber&) = delete;
  auto operator=(const RawSubscriber&) = delete;
  auto operator=(RawSubscriber&&) noexcept = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::ipc
