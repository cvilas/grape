//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#include "grape/ipc/subscriber.h"

#include "ipc_zenoh.h"

namespace {}

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
Subscriber::~Subscriber() = default;

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(std::unique_ptr<zenoh::Subscriber<void>> zs) : impl_(std::move(zs)) {
}
}  // namespace grape::ipc
