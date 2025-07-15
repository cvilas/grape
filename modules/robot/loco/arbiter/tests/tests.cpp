//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <stdexcept>

#include "catch2/catch_test_macros.hpp"
#include "grape/ipc/session.h"
#include "grape/robot/loco/arbiter.h"
#include "grape/robot/loco/topics.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

//=================================================================================================
// Emulates a remote teleop client
class TeleopEmulator {
public:
  using StatusCallback = std::function<void(const grape::robot::loco::ArbiterStatus&)>;
  explicit TeleopEmulator(const std::string& robot_name, StatusCallback&& status_cb);
  void send(const grape::robot::loco::Command& cmd);
  [[nodiscard]] auto isServiceDetected() const -> bool;

private:
  void onStatus(const grape::ipc::Sample& sample);
  StatusCallback status_cb_;
  grape::ipc::Publisher cmd_pub_;
  grape::ipc::Subscriber status_sub_;
};

//-------------------------------------------------------------------------------------------------
TeleopEmulator::TeleopEmulator(const std::string& robot_name, StatusCallback&& status_cb)
  : status_cb_(std::move(status_cb))
  , cmd_pub_(robot_name + grape::robot::loco::AlternateCommandTopic::TOPIC_SUFFIX)
  , status_sub_(robot_name + grape::robot::loco::ArbiterStatusTopic::TOPIC_SUFFIX,
                [this](const auto& st) { onStatus(st); }) {
}

//-------------------------------------------------------------------------------------------------
void TeleopEmulator::send(const grape::robot::loco::Command& cmd) {
  static constexpr auto STREAMBUFFER_SIZE = 1024U;
  auto stream = grape::serdes::OutStream<STREAMBUFFER_SIZE>();
  auto serialiser = grape::serdes::Serialiser(stream);
  if (not serialiser.pack(cmd)) {
    throw std::runtime_error("Failed to pack command");
  }

  cmd_pub_.publish(stream.data());
}

//-------------------------------------------------------------------------------------------------
void TeleopEmulator::onStatus(const grape::ipc::Sample& sample) {
  using Stream = grape::serdes::InStream;
  auto istream = Stream(sample.data);
  auto deserialiser = grape::serdes::Deserialiser(istream);
  auto val = grape::robot::loco::ArbiterStatusTopic::DataType{};
  if (not deserialiser.unpack(val)) {
    throw std::runtime_error("Failed to unpack status");
  }
  status_cb_(val);
}

//-------------------------------------------------------------------------------------------------
auto TeleopEmulator::isServiceDetected() const -> bool {
  return (status_sub_.publisherCount() != 0);
}

//=================================================================================================
TEST_CASE("Locomotion command arbiter behaviours", "[Arbiter]") {
  static constexpr auto* ROBOT_NAME = "test_robot";
  auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Host };
  grape::ipc::init(std::move(ipc_config));
  auto received_cmds = std::vector<grape::robot::loco::Command>{};
  const auto robot_cb = [&received_cmds](const auto& cmd) { received_cmds.push_back(cmd); };
  auto test_service = grape::robot::loco::Arbiter(ROBOT_NAME, robot_cb);
  const auto on_loco_status = [](const auto&) {};
  auto test_client = TeleopEmulator(ROBOT_NAME, on_loco_status);

  // Wait for client-service connection
  constexpr auto RETRY_COUNT = 10U;
  auto count_down = RETRY_COUNT;
  while ((not test_client.isServiceDetected()) && (count_down > 0)) {
    constexpr auto REG_WAIT_TIME = std::chrono::milliseconds(200);
    std::this_thread::sleep_for(REG_WAIT_TIME);
    count_down--;
  }
  REQUIRE(test_client.isServiceDetected());

  received_cmds.clear();

  // initially primary source is routed through
  test_service.setPrimary(grape::robot::loco::KeepAliveCmd{});
  REQUIRE(received_cmds.size() == 1);
  REQUIRE(std::holds_alternative<grape::robot::loco::KeepAliveCmd>(received_cmds.at(0)));

  // Send a teleop command - this should now get through
  static constexpr auto IPC_PROCESSING_DELAY = std::chrono::milliseconds(50);
  test_client.send(grape::robot::loco::Move3DCmd{ .forward_speed = 1.0F });
  std::this_thread::sleep_for(IPC_PROCESSING_DELAY);
  REQUIRE(received_cmds.size() == 2);
  REQUIRE(std::holds_alternative<grape::robot::loco::Move3DCmd>(received_cmds.at(1)));

  // Now primary commands should be ignored if it is sent within teleop timeout period
  test_service.setPrimary(grape::robot::loco::KeepAliveCmd{});
  REQUIRE(received_cmds.size() == 2);  // no new received command

  // But teleop commands should continue to get through
  test_client.send(grape::robot::loco::Move3DCmd{ .lateral_speed = 0.F });
  std::this_thread::sleep_for(IPC_PROCESSING_DELAY);
  REQUIRE(received_cmds.size() == 3);
  REQUIRE(std::holds_alternative<grape::robot::loco::Move3DCmd>(received_cmds.at(2)));

  // A second teleop command should be ignored as long as the first one is active
  auto test_client2 = TeleopEmulator(ROBOT_NAME, on_loco_status);
  test_client2.send(grape::robot::loco::Move3DCmd{ .lateral_speed = 0.F });
  std::this_thread::sleep_for(IPC_PROCESSING_DELAY);
  REQUIRE(received_cmds.size() == 3);

  // But let the first teleop controller timeout and the second will have control
  static constexpr auto TIMEOUT_MARGIN = std::chrono::milliseconds(500);
  std::this_thread::sleep_for(TIMEOUT_MARGIN + grape::robot::loco::Arbiter::ALT_CONTROLLER_TIMEOUT);
  test_client2.send(grape::robot::loco::Move3DCmd{ .lateral_speed = 0.F });
  std::this_thread::sleep_for(IPC_PROCESSING_DELAY);
  REQUIRE(received_cmds.size() == 4);

  // Primary source regains control after teleop controllers times out
  std::this_thread::sleep_for(TIMEOUT_MARGIN + grape::robot::loco::Arbiter::ALT_CONTROLLER_TIMEOUT);
  test_service.setPrimary(grape::robot::loco::KeepAliveCmd{});
  REQUIRE(received_cmds.size() == 5);
  REQUIRE(std::holds_alternative<grape::robot::loco::KeepAliveCmd>(received_cmds.at(4)));
}

}  // namespace
