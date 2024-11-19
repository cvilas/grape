//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#pragma once

#include <functional>
#include <memory>
#include <span>

namespace zenoh {
class Session;
template <class Handler>
class Subscriber;
}  // namespace zenoh

namespace grape::ipc {

class Session;

//-------------------------------------------------------------------------------------------------
using DataCallback = std::function<void(std::span<const std::byte>)>;

//=================================================================================================
/// Subscribers reciver topic data. Created by Session. See also Topic.
class Subscriber {
public:
  ~Subscriber();
  Subscriber(const Subscriber&) = delete;
  auto operator=(const Subscriber&) = delete;
  Subscriber(Subscriber&&) noexcept = delete;
  auto operator=(Subscriber&&) noexcept = delete;

private:
  friend class Session;
  explicit Subscriber(std::unique_ptr<zenoh::Subscriber<void>> zs);
  std::unique_ptr<zenoh::Subscriber<void>> impl_;
};

}  // namespace grape::ipc
