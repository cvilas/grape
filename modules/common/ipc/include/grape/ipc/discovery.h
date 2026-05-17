//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <format>
#include <unordered_map>
#include <vector>

#include "grape/ipc/entity_id.h"

namespace grape::ipc {

struct EndpointInfo {
  EntityId entity_id;
  std::string type_name;
  std::string encoding;
};

[[nodiscard]] constexpr auto toString(const EndpointInfo& ep) -> std::string {
  return std::format("entity_id='{}' type_name='{}' encoding_name='{}'", toString(ep.entity_id),
                     ep.type_name, ep.encoding);
}

struct Endpoints {
  std::vector<EndpointInfo> publishers;
  std::vector<EndpointInfo> subscribers;
};

using TopicName = std::string;

/// @return Active IPC endpoints known to ipc::Session at the time of this call.
[[nodiscard]] auto discover() -> std::unordered_map<TopicName, Endpoints>;

}  // namespace grape::ipc
