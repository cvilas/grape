//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <type_traits>
#include <variant>

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

}  // namespace grape::locomotion
