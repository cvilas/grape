//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc2/publisher.h"

#include <chrono>

#include "publisher_impl.h"

namespace grape::ipc2 {

//-------------------------------------------------------------------------------------------------
Publisher::~Publisher() = default;

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(std::unique_ptr<PublisherImpl> impl) : impl_(std::move(impl)) {
}

//-------------------------------------------------------------------------------------------------
void Publisher::publish(std::span<const std::byte> bytes) const {
  const auto now = std::chrono::system_clock::now();
  const auto us =
      std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
  std::ignore = impl_->pub()->Send(bytes.data(), bytes.size(), us);
}

}  // namespace grape::ipc2
