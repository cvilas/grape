//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/subscriber.h"

#include "subscriber_impl.h"

namespace grape::ipc {
//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(std::unique_ptr<SubscriberImpl> impl) : impl_(std::move(impl)) {
}

//-------------------------------------------------------------------------------------------------
Subscriber::~Subscriber() = default;

//-------------------------------------------------------------------------------------------------
auto Subscriber::getPublisherCount() const -> std::size_t {
  return impl_->sub()->GetPublisherCount();
}
}  // namespace grape::ipc
