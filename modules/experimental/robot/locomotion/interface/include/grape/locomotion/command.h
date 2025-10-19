//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/locomotion/keep_alive_cmd.h"
#include "grape/locomotion/move_3d_cmd.h"

namespace grape::locomotion {

using Command = std::variant<KeepAliveCmd, Move3DCmd>;

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Command& cmd) -> std::string {
  return std::visit([](const auto& var) -> std::string { return toString(var); }, cmd);
}

}  // namespace grape::locomotion
