//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/locomotion/teleop_client.h"

namespace grape::locomotion {

//-------------------------------------------------------------------------------------------------
TeleopClient::TeleopClient(const std::string& robot_name, StatusCallback&& status_cb)
  : status_cb_(std::move(status_cb))
  , arbiter_status_sub_(
        ArbiterStatusTopic(robot_name),
        [this](const auto& st, const auto& info) { onArbiterStatus(st, info); },
        [this](const auto& match) { onArbiterMatch(match); })
  , cmd_pub_(AlternateCommandTopic(robot_name)) {
  id_ = cmd_pub_.id();
}

//-------------------------------------------------------------------------------------------------
auto TeleopClient::send(const AlternateCommandTopic::DataType& cmd) -> bool {
  return cmd_pub_.publish(cmd).has_value();
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::onArbiterStatus(const std::expected<ArbiterStatus, ipc::Error>& maybe_status,
                                   const ipc::SampleInfo& info) const {
  (void)info;
  if (status_cb_ == nullptr) {
    return;
  }
  if (maybe_status) {
    const auto& status = maybe_status.value();
    status_cb_(ClientStatus{ .is_client_active = (status.alt_controller_id == id_),
                             .command_latency = status.alt_command_latency });
  } else {
    const auto err_msg = toString(maybe_status.error());
    status_cb_(Error{ .message = std::format("Invalid Arbiter status: {}", err_msg) });
  }
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::onArbiterMatch(const ipc::Match& match) const {
  switch (match.status) {
    case ipc::Match::Status::Unmatched:
      if (arbiter_status_sub_.publisherCount() == 0) {
        status_cb_(ServiceStatus{ .is_detected = false });
      }
      break;
    case ipc::Match::Status::Matched:
      status_cb_(ServiceStatus{ .is_detected = true });
      break;
  }
}
}  // namespace grape::locomotion
