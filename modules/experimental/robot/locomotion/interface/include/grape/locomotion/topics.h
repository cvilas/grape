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
  static constexpr auto QOS = ipc::QoS::BestEffort;
  static constexpr auto SERDES_BUFFER_SIZE = 128U;
  explicit AlternateCommandTopic(const std::string& robot_name)
    : topic_name_(robot_name + "/locomotion/command/alternate") {
  }
  [[nodiscard]] auto topicName() const -> const std::string& {
    return topic_name_;
  }

private:
  std::string topic_name_;
};

/// Topic on which the locomotion service publishes the status of the robot's locomotion system.
class ArbiterStatusTopic {
public:
  using DataType = locomotion::ArbiterStatus;
  static constexpr auto QOS = ipc::QoS::BestEffort;
  static constexpr auto SERDES_BUFFER_SIZE = 128U;
  explicit ArbiterStatusTopic(const std::string& robot_name)
    : topic_name_(robot_name + "/locomotion/arbiter/status") {
  }
  [[nodiscard]] auto topicName() const -> const std::string& {
    return topic_name_;
  }

private:
  std::string topic_name_;
};

}  // namespace grape::locomotion
