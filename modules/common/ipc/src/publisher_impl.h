//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <ecal/pubsub/publisher.h>

namespace grape::ipc {

class PublisherImpl {
public:
  explicit PublisherImpl(std::unique_ptr<eCAL::CPublisher> pb) : pub_(std::move(pb)) {
  }
  [[nodiscard]] auto pub() const -> eCAL::CPublisher* {
    return pub_.get();
  }

private:
  std::unique_ptr<eCAL::CPublisher> pub_;
};

}  // namespace grape::ipc
