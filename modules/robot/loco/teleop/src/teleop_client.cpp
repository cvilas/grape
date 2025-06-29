//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/robot/loco/teleop_client.h"

#include "grape/log/syslog.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace grape::robot::loco {
//-------------------------------------------------------------------------------------------------
TeleopClient::TeleopClient(const std::string& robot_name, StatusCallback&& status_cb)
  : status_cb_(std::move(status_cb))
  , loco_status_sub_(
        robot_name + loco::StatusTopic::TOPIC_SUFFIX, [this](const auto& st) { onLocoStatus(st); },
        [this](const auto& mch) { onServerMatch(mch); })
  , cmd_pub_(robot_name + loco::TeleopCommandTopic::TOPIC_SUFFIX) {
  id_ = cmd_pub_.id();
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::send(const loco::TeleopCommandTopic::DataType& cmd) {
  static constexpr auto STREAMBUFFER_SIZE = 128U;
  auto stream = serdes::OutStream<STREAMBUFFER_SIZE>();
  auto serialiser = serdes::Serialiser(stream);
  if (not serialiser.pack(cmd)) {
    syslog::Error("Failed to pack command");
    return;
  }

  cmd_pub_.publish(stream.data());
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::onLocoStatus(const ipc::Sample& sample) const {
  using Stream = serdes::InStream;
  auto istream = Stream(sample.data);
  auto deserialiser = serdes::Deserialiser(istream);
  auto loco_status = loco::StatusTopic::DataType{};
  if (not deserialiser.unpack(loco_status)) {
    syslog::Error("Failed to unpack status");
    return;
  }
  const auto teleop_status =
      TeleopClient::Status{ .is_service_detected = true,
                            .is_client_active = (loco_status.teleop_controller_id == id_),
                            .command_latency = loco_status.teleop_command_latency };
  if (status_cb_ != nullptr) {
    status_cb_(teleop_status);
  }
}

//-------------------------------------------------------------------------------------------------
void TeleopClient::onServerMatch(const ipc::Match& match) const {
  switch (match.status) {
    case ipc::Match::Status::Unmatched:
      if (loco_status_sub_.publisherCount() == 0) {
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
}  // namespace grape::robot::loco
