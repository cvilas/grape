//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#include "grape/ipc/publisher.h"

#include "ipc_zenoh.h"

namespace grape::ipc {
//-------------------------------------------------------------------------------------------------
Publisher::~Publisher() = default;

//-------------------------------------------------------------------------------------------------
void Publisher::publish(std::span<const char> bytes) {
  const auto bytes_view = std::string_view(bytes.data(), bytes.size());
  impl_->put(bytes_view);
}

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(std::unique_ptr<zenoh::Publisher> zp) : impl_(std::move(zp)) {
}
}  // namespace grape::ipc
