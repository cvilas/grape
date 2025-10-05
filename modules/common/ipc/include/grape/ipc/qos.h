//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>

#include "grape/utils/enums.h"

namespace grape::ipc {

/// Quality of service (QoS) for data communication over a topic
///
/// BestEffort QoS is suitable for transporting data of _any_ size between processes on the same
/// host, and for small data (kilobytes) across hosts on the network, all with low latency and low
/// network overheads
/// Reliable QoS is specifically useful for transporting large data (> 1MB) _across_ hosts on a
/// network. Avoid using this for anything else due to the higher latency and network overheads
enum class QoS : std::uint8_t { BestEffort, Reliable };

[[nodiscard]] constexpr auto toString(QoS qos) -> std::string_view {
  return grape::enums::name(qos);
}

}  // namespace grape::ipc
