//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>
#include <span>

namespace grape::ipc2 {

class Session;
class PublisherImpl;

//=================================================================================================
/// Publishers post topic data. Created by Session. See also Topic.
class Publisher {
public:
  /// Publish data on topic specified at creation by Session
  void publish(std::span<const std::byte> bytes) const;

  ~Publisher();
  Publisher(const Publisher&) = delete;
  auto operator=(const Publisher&) = delete;
  Publisher(Publisher&&) noexcept = default;
  auto operator=(Publisher&&) noexcept = delete;

private:
  friend class Session;
  explicit Publisher(std::unique_ptr<PublisherImpl> impl);
  std::unique_ptr<PublisherImpl> impl_;
};

}  // namespace grape::ipc2
