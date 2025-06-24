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
  : robot_command_cb_(std::move(robot_cmd_cb)) {
  status_pub_ = std::make_unique<ipc::Publisher>(robot_name + StatusTopic::TOPIC_SUFFIX, nullptr);

  teleop_sub_ =
      std::make_unique<ipc::Subscriber>(robot_name + TeleopCommandTopic::TOPIC_SUFFIX,
                                        [this](const ipc::Sample& data) { onTeleopCommand(data); });

  autonav_sub_ = std::make_unique<ipc::Subscriber>(
      robot_name + AutonavCommandTopic::TOPIC_SUFFIX,
      [this](const ipc::Sample& data) { onAutonavCommand(data); });

  watchdog_thread_ = std::jthread([this](const std::stop_token& token) { watchdogLoop(token); });
}

//-------------------------------------------------------------------------------------------------
Service::~Service() = default;

//-------------------------------------------------------------------------------------------------
void Service::onTeleopCommand(const ipc::Sample& sample) {
  if (teleoperator_id_.load() == INVALID_ID) {
    // no teleoperator active. take control
    teleoperator_id_.store(sample.publisher.id);
    grape::syslog::Note("Teleoperator '{}' is taking control", toString(sample.publisher));
    publishStatus();
  }

  if (teleoperator_id_.load() != sample.publisher.id) {
    // message not from controlling teleoperator. ignore
    return;
  }

  // process command
  last_teleop_cmd_time_.store(sample.publish_time);
  const auto cmd = unpack<TeleopCommandTopic::DataType>(sample.data);
  if (not cmd.has_value()) {
    grape::syslog::Warn("Failed to unpack teleop command from '{}'", toString(sample.publisher));
    return;
  }
  robot_command_cb_(*cmd);
}

//-------------------------------------------------------------------------------------------------
void Service::onAutonavCommand(const ipc::Sample& sample) {
  if (teleoperator_id_.load() != INVALID_ID) {
    // Teleop active. ignore.
    return;
  }

  // process command
  const auto cmd = unpack<AutonavCommandTopic::DataType>(sample.data);
  if (not cmd.has_value()) {
    grape::syslog::Warn("Failed to unpack autonav command");
    return;
  }
  robot_command_cb_(*cmd);
}

//-------------------------------------------------------------------------------------------------
void Service::watchdogLoop(const std::stop_token& stop_token) {
  while (not stop_token.stop_requested()) {
    std::this_thread::sleep_for(WATCHDOG_INTERVAL);

    const auto teleop_id = teleoperator_id_.load();
    if (teleop_id == INVALID_ID) {
      continue;
    }

    const auto now = std::chrono::system_clock::now();
    const auto time_since_last_teleop = now - last_teleop_cmd_time_.load();

    if (time_since_last_teleop > TELEOP_TIMEOUT) {
      teleoperator_id_.store(INVALID_ID);
      grape::syslog::Note("Teleoperator '{:#x}' timed out, autonav can take control", teleop_id);
      publishStatus();
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Service::publishStatus() const {
  const auto status = Status{ .teleop_controller_id = teleoperator_id_.load() };

  static constexpr auto STREAMBUFFER_SIZE = 1024U;
  auto stream = serdes::OutStream<STREAMBUFFER_SIZE>();
  auto serialiser = serdes::Serialiser(stream);
  if (not serialiser.pack(status)) {
    grape::syslog::Error("Failed to pack status");
    return;
  }
  status_pub_->publish(stream.data());
}

}  // namespace grape::robot::loco
