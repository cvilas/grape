//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <span>

namespace grape::ipc {

class Session;
class SubscriberImpl;

//=================================================================================================
/// Defines data sample received by the subscriber, with related meta-information
struct Sample {
  std::span<const std::byte> data;
  std::chrono::system_clock::time_point publish_time;
};

//=================================================================================================
/// Subscribers receive topic data. Created by Session. See also Topic.
class Subscriber {
public:
  /// Function signature for callback on received data
  using DataCallback = std::function<void(const Sample&)>;

  /// @return The number of publishers currently matched to this subscriber
  [[nodiscard]] auto getPublisherCount() const -> std::size_t;

  ~Subscriber();
  Subscriber(const Subscriber&) = delete;
  auto operator=(const Subscriber&) = delete;
  Subscriber(Subscriber&&) noexcept = default;
  auto operator=(Subscriber&&) noexcept = delete;

private:
  friend class Session;
  explicit Subscriber(std::unique_ptr<SubscriberImpl> impl);
  std::unique_ptr<SubscriberImpl> impl_;
};

}  // namespace grape::ipc
