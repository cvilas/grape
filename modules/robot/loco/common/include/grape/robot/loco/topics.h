//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/robot/loco/types.h"

namespace grape::robot::loco {

/// Topic on which a secondary/teleop client publishes commands intended for the locomotion service
struct TeleopCommandTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/teleop/command";
  using DataType = robot::loco::Command;
};

/// Topic on which the locomotion service publishes the status of the robot's locomotion system.
struct StatusTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/status";
  using DataType = robot::loco::Status;
};

}  // namespace grape::robot::loco
