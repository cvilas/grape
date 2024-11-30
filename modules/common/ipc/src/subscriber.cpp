//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/subscriber.h"

#include "ipc_zenoh.h"  // should be included before zenoh headers

namespace {}

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
Subscriber::~Subscriber() = default;

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(std::unique_ptr<zenoh::Subscriber<void>> zs) : impl_(std::move(zs)) {
}
}  // namespace grape::ipc
