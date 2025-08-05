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
class ArbiterStatusTopic {
public:
  using DataType = robot::loco::ArbiterStatus;
  static constexpr auto MAX_SERIALISED_DATA_SIZE = 128U;
  explicit ArbiterStatusTopic(std::string robot_name) : robot_name_(std::move(robot_name)) {
  }
  [[nodiscard]] auto topic() const -> std::string {
    static constexpr auto TOPIC_SUFFIX = "/loco/arbiter/status";
    return robot_name_ + TOPIC_SUFFIX;
  }

private:
  std::string robot_name_;
};

}  // namespace grape::robot::loco
