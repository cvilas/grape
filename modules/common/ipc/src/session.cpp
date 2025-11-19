//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include <atomic>
#include <cstdlib>

#include <ecal/config/configuration.h>
#include <ecal/core.h>

#include "grape/exception.h"

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<bool> s_initialized = false;

}  // namespace

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
void init(const Config& config) {
  bool expected = false;
  if (not s_initialized.compare_exchange_strong(expected, true)) {
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
  if (not s_initialized) {
    panic("Not initialised. Call init() first.");
  }
  return eCAL::Ok();
}

}  // namespace grape::ipc
