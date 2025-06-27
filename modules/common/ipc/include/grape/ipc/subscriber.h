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
/// Defines data sample received by the subscriber, with related meta-information
struct Sample {
  std::span<const std::byte> data;
  std::chrono::system_clock::time_point publish_time;
  EntityId publisher;
};

//=================================================================================================
/// Subscribers receive topic data. Created by Session. See also Topic.
class Subscriber {
public:
  /// Function signature for callback on received data
  using DataCallback = std::function<void(const Sample&)>;

  /// creates a subscriber
  /// @param topic Topic on which to listen to for data from matched publishers
  /// @param data_cb Data processing callback, triggered on every newly received data sample
  /// @param match_cb Match callback, triggered when matched/unmatched with a remote publisher
  Subscriber(const std::string& topic, DataCallback&& data_cb, MatchCallback&& match_cb = nullptr);

  /// @return The number of publishers currently matched to this subscriber
  [[nodiscard]] auto getPublisherCount() const -> std::size_t;

  ~Subscriber();
  Subscriber(Subscriber&&) noexcept;
  Subscriber(const Subscriber&) = delete;
  auto operator=(const Subscriber&) = delete;
  auto operator=(Subscriber&&) noexcept = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::ipc
