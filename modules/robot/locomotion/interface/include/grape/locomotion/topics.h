//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/locomotion/arbiter_status.h"
#include "grape/locomotion/command.h"

namespace grape::locomotion {

/// Topic on which the alternate source publishes commands intended for the locomotion stack
class AlternateCommandTopic {
public:
  using DataType = locomotion::Command;
  static constexpr auto TOPIC_SUFFIX = "/locomotion/command/alternate";
  static constexpr auto SERDES_BUFFER_SIZE = 128U;
  explicit AlternateCommandTopic(std::string robot_name) : robot_name_(std::move(robot_name)) {
  }
  [[nodiscard]] auto topicName() const -> std::string {
    return robot_name_ + TOPIC_SUFFIX;
  }

private:
  std::string robot_name_;
};

/// Topic on which the locomotion service publishes the status of the robot's locomotion system.
class ArbiterStatusTopic {
public:
  using DataType = locomotion::ArbiterStatus;
  static constexpr auto TOPIC_SUFFIX = "/locomotion/arbiter/status";
  static constexpr auto SERDES_BUFFER_SIZE = 128U;
  explicit ArbiterStatusTopic(std::string robot_name) : robot_name_(std::move(robot_name)) {
  }
  [[nodiscard]] auto topicName() const -> std::string {
    return robot_name_ + TOPIC_SUFFIX;
  }

private:
  std::string robot_name_;
};

}  // namespace grape::locomotion
