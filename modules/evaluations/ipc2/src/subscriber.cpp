//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/ipc2/subscriber.h"

#include "ipc_zenoh.h"  // should be included before zenoh headers

namespace {}

namespace grape::ipc2 {

//-------------------------------------------------------------------------------------------------
Subscriber::~Subscriber() = default;

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(std::unique_ptr<zenoh::Subscriber<void>> zs) : impl_(std::move(zs)) {
}
}  // namespace grape::ipc2
