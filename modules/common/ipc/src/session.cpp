//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include <optional>

#include <ecal/config/configuration.h>
#include <ecal/core.h>

#include "grape/exception.h"

namespace {

struct Manager {
  explicit Manager(grape::ipc::Config&& config) {
    auto grape_config = std::move(config);
    auto ecal_config = eCAL::Init::Configuration();
    switch (grape_config.scope) {
      case grape::ipc::Config::Scope::Host:
        ecal_config.communication_mode = eCAL::eCommunicationMode::local;
        break;
      case grape::ipc::Config::Scope::Network:
        ecal_config.communication_mode = eCAL::eCommunicationMode::network;
        ecal_config.publisher.layer.tcp.enable = true;
        ecal_config.publisher.layer.udp.enable = false;
        ecal_config.subscriber.layer.tcp.enable = true;
        ecal_config.subscriber.layer.udp.enable = false;
        break;
    }
    eCAL::Initialize(ecal_config, grape_config.name);
  }

  ~Manager() {
    eCAL::Finalize();
  }

  Manager(Manager const&) = delete;
  Manager(Manager&&) = delete;
  void operator=(Manager const&) = delete;
  void operator=(Manager&&) = delete;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::optional<Manager> s_manager = std::nullopt;

}  // namespace

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
void init(Config&& config) {
  if (s_manager.has_value()) {
    panic<Exception>("Already initialised");
  }
  s_manager.emplace(std::move(config));
}

//-------------------------------------------------------------------------------------------------
auto ok() -> bool {
  if (not s_manager) {
    panic<Exception>("Not initialised. Call init() first.");
  }
  return eCAL::Ok();
}

}  // namespace grape::ipc
