//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <ecal/config/configuration.h>

namespace grape::ipc {

inline auto defaultConfig() -> eCAL::Configuration {
  auto config = eCAL::Configuration{};
  config.publisher.layer.shm.enable = true;
  config.publisher.layer.udp.enable = true;
  config.publisher.layer.tcp.enable = true;

  config.subscriber.layer.shm.enable = true;
  config.subscriber.layer.udp.enable = true;
  config.subscriber.layer.tcp.enable = true;

#ifdef __APPLE__
  // Cannot use shm transport on macOS due to shm name length and flock support limitations
  config.publisher.layer.shm.enable = false;
  config.subscriber.layer.shm.enable = false;
  config.publisher.layer_priority_local = {
    eCAL::TransportLayer::eType::udp_mc,
    eCAL::TransportLayer::eType::tcp,
  };
#endif
  return config;
}
}  // namespace grape::ipc
