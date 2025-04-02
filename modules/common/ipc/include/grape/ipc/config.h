//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/utils/utils.h"

namespace grape::ipc {

/// IPC session configuration parameters
struct Config {
  /// Operating scope of publishers and subscribers in the session
  enum class Scope : std::uint8_t {
    Host,    //!< Messages confined to host
    Network  //!< Messages can be exchanged across LAN
  };
  std::string name = utils::getProgramName();  //!< user-defined identifier
  Scope scope = Scope::Host;
};

}  // namespace grape::ipc
