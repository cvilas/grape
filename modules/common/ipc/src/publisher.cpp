//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/publisher.h"

#include "ipc_zenoh.h"  // should be included before zenoh headers

namespace grape::ipc {
//-------------------------------------------------------------------------------------------------
Publisher::~Publisher() = default;

//-------------------------------------------------------------------------------------------------
void Publisher::publish(std::span<const std::byte> bytes) {
  const auto bytes_view = std::string_view(
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(bytes.data()),  //
      bytes.size_bytes());
  impl_->put(bytes_view);
}

//-------------------------------------------------------------------------------------------------
void Publisher::publish(std::span<const char> bytes) {
  impl_->put(std::string_view{ bytes.data(), bytes.size() });
}

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(std::unique_ptr<zenoh::Publisher> zp) : impl_(std::move(zp)) {
}
}  // namespace grape::ipc
