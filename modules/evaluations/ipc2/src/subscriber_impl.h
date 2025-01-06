//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <ecal/pubsub/subscriber.h>

namespace grape::ipc2 {

class SubscriberImpl {
public:
  explicit SubscriberImpl(std::unique_ptr<eCAL::CSubscriber> sb) : sub_(std::move(sb)) {
  }
  [[nodiscard]] auto sub() const -> eCAL::CSubscriber* {
    return sub_.get();
  }

private:
  std::unique_ptr<eCAL::CSubscriber> sub_;
};

}  // namespace grape::ipc2
