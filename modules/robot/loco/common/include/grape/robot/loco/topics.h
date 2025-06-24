//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/robot/loco/types.h"

namespace grape::robot::loco {

struct AutonavCommandTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/autonav/command";
  using DataType = robot::loco::Command;
};

struct TeleopCommandTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/teleop/command";
  using DataType = robot::loco::Command;
};

struct StatusTopic {
  static constexpr auto TOPIC_SUFFIX = "/loco/status";
  using DataType = robot::loco::Status;
};

}  // namespace grape::robot::loco
