//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/robot/loco/service.h"

#include "grape/log/syslog.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

//-------------------------------------------------------------------------------------------------
template <typename T>
auto unpack(std::span<const std::byte> bytes) -> std::optional<T> {
  using Stream = grape::serdes::InStream;
  auto istream = Stream(bytes);
  auto deserialiser = grape::serdes::Deserialiser(istream);
  auto val = T{};
  if (not deserialiser.unpack(val)) {
    return {};
  }
  return val;
}

}  // namespace

namespace grape::robot::loco {

//=================================================================================================
Service::Service(const std::string& robot_name, CommandCallback&& robot_cmd_cb)
  : robot_command_cb_(std::move(robot_cmd_cb))
  , status_pub_(robot_name + StatusTopic::TOPIC_SUFFIX, nullptr)
  , teleop_sub_(robot_name + TeleopCommandTopic::TOPIC_SUFFIX,
                [this](const auto& data) { onTeleopCommand(data); })
  , watchdog_thread_([this](const std::stop_token& token) { watchdogLoop(token); }) {
}

//-------------------------------------------------------------------------------------------------
Service::~Service() = default;

//-------------------------------------------------------------------------------------------------
void Service::onTeleopCommand(const ipc::Sample& sample) {
  if (teleoperator_id_.load() == NULL_ID) {
    // no teleoperator active. take control
    teleoperator_id_.store(sample.publisher.id);
    grape::syslog::Note("Teleoperator '{}' has control", toString(sample.publisher));
    publishStatus();
  }

  if (teleoperator_id_.load() != sample.publisher.id) {
    // message not from controlling teleoperator. ignore
    return;
  }

  const auto now = std::chrono::system_clock::now();
  const auto cmd_latency = std::chrono::duration<float>(now - sample.publish_time);
  teleop_cmd_latency_ = teleop_cmd_latency_tracker_.append(cmd_latency.count()).mean;

  // process command
  last_teleop_cmd_time_.store(now);
  const auto cmd = unpack<TeleopCommandTopic::DataType>(sample.data);
  if (not cmd.has_value()) {
    grape::syslog::Warn("Failed to unpack teleop command from '{}'", toString(sample.publisher));
    return;
  }
  robot_command_cb_(*cmd);
}

//-------------------------------------------------------------------------------------------------
void Service::send(const loco::Command& cmd) const {
  if (teleoperator_id_.load() != NULL_ID) {
    return;  // Teleop is active. ignore this command.
  }

  // pass the command through
  robot_command_cb_(cmd);
}

//-------------------------------------------------------------------------------------------------
void Service::watchdogLoop(const std::stop_token& stop_token) {
  static constexpr auto STATUS_UPDATE_INTERVAL = std::chrono::milliseconds(1000);
  static constexpr auto TELEOP_CHECK_INTERVAL = STATUS_UPDATE_INTERVAL / 2;
  auto last_status_update_time = std::chrono::system_clock::now();
  while (not stop_token.stop_requested()) {
    std::this_thread::sleep_for(TELEOP_CHECK_INTERVAL);

    const auto teleop_id = teleoperator_id_.load();
    if (teleop_id == NULL_ID) {
      continue;  // only monitor when teleop is active
    }

    const auto now = std::chrono::system_clock::now();
    auto publish_status = false;

    if (now - last_teleop_cmd_time_.load() > TELEOP_TIMEOUT) {
      teleoperator_id_.store(NULL_ID);
      grape::syslog::Note("Teleoperator '{:#x}' timed out, autonav has control", teleop_id);
      teleop_cmd_latency_.store(0.F);
      publish_status = true;  // notify status change
    }

    if (now - last_status_update_time > STATUS_UPDATE_INTERVAL) {
      // periodically update when teleop is active
      publish_status = true;
    }

    if (publish_status) {
      last_status_update_time = now;
      publishStatus();
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Service::publishStatus() const {
  const auto avg_latency = std::chrono::duration_cast<std::chrono::system_clock::duration>(
      std::chrono::duration<float>(teleop_cmd_latency_.load()));
  const auto status = Status{ .teleop_controller_id = teleoperator_id_.load(),
                              .teleop_command_latency = avg_latency };

  static constexpr auto STREAMBUFFER_SIZE = 128U;
  auto stream = serdes::OutStream<STREAMBUFFER_SIZE>();
  auto serialiser = serdes::Serialiser(stream);
  if (not serialiser.pack(status)) {
    grape::syslog::Error("Failed to pack status");
    return;
  }
  status_pub_.publish(stream.data());
}

}  // namespace grape::robot::loco
