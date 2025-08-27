//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/locomotion/teleop_client.h"

#include "grape/log/syslog.h"

namespace grape::locomotion {

//-------------------------------------------------------------------------------------------------
TeleopClient::TeleopClient(const std::string& robot_name, StatusCallback&& status_cb)
  : status_cb_(std::move(status_cb))
  , arbiter_status_sub_(
        ArbiterStatusTopic(robot_name),
        [this](const auto& st, const auto& info) -> void { onArbiterStatus(st, info); },
        [this](const auto& match) -> void { onArbiterMatch(match); })
  , cmd_pub_(AlternateCommandTopic(robot_name)) {
  id_ = cmd_pub_.id();
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::send(const AlternateCommandTopic::DataType& cmd) {
  cmd_pub_.publish(cmd);
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::onArbiterStatus(const ArbiterStatus& status, const ipc::SampleInfo& info) const {
  (void)info;
  const auto teleop_status =
      TeleopClient::Status{ .is_service_detected = true,
                            .is_client_active = (status.alt_controller_id == id_),
                            .command_latency = status.alt_command_latency };
  if (status_cb_ != nullptr) {
    status_cb_(teleop_status);
  }
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::onArbiterMatch(const ipc::Match& match) const {
  switch (match.status) {
    case ipc::Match::Status::Unmatched:
      if (arbiter_status_sub_.publisherCount() == 0) {
        status_cb_({ .is_service_detected = false });  // notify client that nobody is listening
      }
      break;
    case ipc::Match::Status::Matched:
      status_cb_({ .is_service_detected = true });
      break;
    case ipc::Match::Status::Undefined:
      break;
  }
}
}  // namespace grape::locomotion
