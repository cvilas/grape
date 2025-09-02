//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/utils/enums.h"

namespace grape::ipc {

/// IPC error codes
enum class Error : std::uint8_t {
  SerialisationFailed,   //!< Error serialising data before publishing
  PublishFailed,         //!< Error writing data to the transport layer
  DeserialisationFailed  //!< Error deserialising data in the subscriber
};

[[nodiscard]] constexpr auto toString(Error error) -> std::string_view {
  return enums::name(error);
}

}  // namespace grape::ipc
