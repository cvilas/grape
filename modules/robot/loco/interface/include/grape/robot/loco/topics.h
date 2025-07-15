//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/robot/loco/arbiter_status.h"
#include "grape/robot/loco/command.h"

namespace grape::robot::loco {

/// Topic on which the alternate source publishes commands intended for the locomotion stack
struct AlternateCommandTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/command/alternate";
  using DataType = robot::loco::Command;
};

/// Topic on which the locomotion service publishes the status of the robot's locomotion system.
struct ArbiterStatusTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/arbiter/status";
  using DataType = robot::loco::ArbiterStatus;
};

}  // namespace grape::robot::loco
