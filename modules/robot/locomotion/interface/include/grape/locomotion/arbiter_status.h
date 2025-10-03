//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <type_traits>
#include <variant>

#include "grape/serdes/serdes.h"

namespace grape::locomotion {

//=================================================================================================
/// Status of the arbiter component in the robot GNC pipeline
struct ArbiterStatus {
  std::uint64_t alt_controller_id{ 0UL };
  WallClock::Duration alt_command_latency{ 0 };  //!<
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const ArbiterStatus& cmd) -> std::string {
  return std::format("ArbiterStatus{{alt_controller_id={:#x}, alt_command_latency={}}}",
                     cmd.alt_controller_id, cmd.alt_command_latency.count());
}

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
[[nodiscard]] auto serialise(serdes::Serialiser<S>& ser, const ArbiterStatus& data) -> bool {
  return ser.pack(data.alt_controller_id) and ser.pack(data.alt_command_latency.count());
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
[[nodiscard]] auto deserialise(serdes::Deserialiser<S>& des, ArbiterStatus& data) -> bool {
  if (not des.unpack(data.alt_controller_id)) {
    return false;
  }
  WallClock::Duration::rep ticks{};
  if (not des.unpack(ticks)) {
    return false;
  }
  data.alt_command_latency = WallClock::Duration(ticks);
  return true;
}

}  // namespace grape::locomotion
