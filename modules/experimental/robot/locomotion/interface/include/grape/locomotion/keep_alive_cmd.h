//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <string>

namespace grape::locomotion {

//=================================================================================================
/// A no-op locomotion command to keep connection stream alive
///
/// Command prevents a connection from timing out when no other locomotion commands are sent.
/// Expected to perform no action on the robot.
struct KeepAliveCmd {};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const KeepAliveCmd& cmd) -> std::string {
  (void)cmd;
  return "KeepAliveCmd{}";
}

}  // namespace grape::locomotion
