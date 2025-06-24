//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <print>
#include <thread>

#include "grape/app/app.h"
#include "grape/conio/conio.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/robot/loco/topics.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {
//=================================================================================================
// Emulates a robot locomotion controller
class Controller {
public:
  enum class Type : std::uint8_t { Autonav, Teleop };
  explicit Controller(const std::string& robot_name, Type type);
  void send(const grape::robot::loco::Command& cmd);
  [[nodiscard]] auto id() const -> std::uint64_t;

private:
  grape::ipc::Publisher cmd_publisher_;
};

//=================================================================================================
// Listens to status updates from locomotion service
class StatusListener {
public:
  explicit StatusListener(const std::string& robot_name, std::uint64_t teleop_id);

private:
  void onStatus(const grape::ipc::Sample& sample) const;
  std::uint64_t teleop_id_;
  grape::ipc::Subscriber status_subscriber_;
};

//-------------------------------------------------------------------------------------------------
auto topic(Controller::Type type, const std::string& robot_name) -> std::string {
  switch (type) {
    case Controller::Type::Autonav:
      return robot_name + grape::robot::loco::AutonavCommandTopic::TOPIC_SUFFIX;
    case Controller::Type::Teleop:
      return robot_name + grape::robot::loco::TeleopCommandTopic::TOPIC_SUFFIX;
  }
  return "";
}
}  // namespace

//=================================================================================================
// - Creates two controllers, one mimics autonomous nav controller and the other teleop controller.
// - Creates a listener for status messages from robot locomotion stack
// - Demonstrates switching behaviour between autonav and teleop controllers
// - Paired with grape_robot_loco_server_example.cpp
//
auto main(int argc, const char* argv[]) -> int {  // NOLINT(bugprone-exception-escape)
  grape::app::init(argc, argv, "GRAPE dummy robot teleop client example");

  static constexpr auto ROBOT_NAME = "dummy_robot";
  auto autonav_commander = Controller(ROBOT_NAME, Controller::Type::Autonav);
  auto teleop_commander = Controller(ROBOT_NAME, Controller::Type::Teleop);
  auto status_listener = StatusListener(ROBOT_NAME, teleop_commander.id());

  auto is_teleop = false;
  static constexpr auto CONTROL_PERIOD = std::chrono::milliseconds(200);
  while (grape::app::ok()) {
    if (grape::conio::kbhit()) {
      const auto key = grape::conio::getch();
      if (key == 't') {
        is_teleop = !is_teleop;
        std::println("Teleop {}", (is_teleop ? "requested" : "request cancelled"));
      }
    }

    const auto cmd = grape::robot::loco::Move3DCmd{ .forward_speed = 0.5F,
                                                    .lateral_speed = 0.0F,
                                                    .turn_speed = 0.1F };

    // simulate autonomous navigation stack always being active
    autonav_commander.send(cmd);

    // telelop stack should override autonomous navigation when active (on the server side)
    if (is_teleop) {
      teleop_commander.send(cmd);
    }

    std::this_thread::sleep_for(CONTROL_PERIOD);
  }

  return EXIT_SUCCESS;
}

namespace {
//-------------------------------------------------------------------------------------------------
Controller::Controller(const std::string& robot_name, Type type)
  : cmd_publisher_(topic(type, robot_name)) {
}

//-------------------------------------------------------------------------------------------------
void Controller::send(const grape::robot::loco::Command& cmd) {
  static constexpr auto STREAMBUFFER_SIZE = 1024U;
  auto stream = grape::serdes::OutStream<STREAMBUFFER_SIZE>();
  auto serialiser = grape::serdes::Serialiser(stream);
  if (not serialiser.pack(cmd)) {
    std::println("Failed to pack command");
    return;
  }

  cmd_publisher_.publish(stream.data());
}

//-------------------------------------------------------------------------------------------------
auto Controller::id() const -> std::uint64_t {
  return cmd_publisher_.id();
}

//-------------------------------------------------------------------------------------------------
StatusListener::StatusListener(const std::string& robot_name, std::uint64_t teleop_id)
  : teleop_id_(teleop_id)
  , status_subscriber_(robot_name + grape::robot::loco::StatusTopic::TOPIC_SUFFIX,
                       [this](const auto& st) { onStatus(st); }) {
}

//-------------------------------------------------------------------------------------------------
void StatusListener::onStatus(const grape::ipc::Sample& sample) const {
  using Stream = grape::serdes::InStream;
  auto istream = Stream(sample.data);
  auto deserialiser = grape::serdes::Deserialiser(istream);
  auto val = grape::robot::loco::StatusTopic::DataType{};
  if (not deserialiser.unpack(val)) {
    std::println("Failed to unpack status");
    return;
  }
  std::println("Teleop {}", (val.teleop_controller_id == teleop_id_) ? "active" : "inactive");
}

}  // namespace
