//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include <ecal/config/configuration.h>
#include <ecal/core.h>

#include "grape/exception.h"

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
void init(const Config& config) {
  if (eCAL::Ok()) {
    panic("Already initialised");
  }

  auto ecal_config = eCAL::Init::Configuration();
  switch (config.scope) {
    case Config::Scope::Host:
      ecal_config.communication_mode = eCAL::eCommunicationMode::local;
      break;
    case Config::Scope::Network:
      ecal_config.communication_mode = eCAL::eCommunicationMode::network;
      break;
  }
  eCAL::Initialize(ecal_config, config.name);
  std::ignore = std::atexit([]() { eCAL::Finalize(); });
}

//-------------------------------------------------------------------------------------------------
auto ok() -> bool {
  return eCAL::Ok();
}

}  // namespace grape::ipc
