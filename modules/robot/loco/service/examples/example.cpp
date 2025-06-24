//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <print>

#include "grape/app/app.h"
#include "grape/robot/loco/service.h"

namespace {

//=================================================================================================
class DummyRobot {
public:
  void move(const grape::robot::loco::Command& cmd) {
    (void)this;
    (void)cmd;
    // std::visit([](const auto& val) { std::println("[robot] {}", toString(val)); }, cmd);
  }
};

}  // namespace

//=================================================================================================
// - Creates a dummy robot and attaches a locomotion service to it
// - Paired with grape_robot_loco_server_client_example.cpp to demonstrate service behaviour
//
auto main(int argc, const char* argv[]) -> int {  // NOLINT(bugprone-exception-escape)
  grape::app::init(argc, argv, "GRAPE dummy robot locomotion service example");

  // Create a dummy robot platform and hook loco service to it
  static constexpr auto ROBOT_NAME = "dummy_robot";
  auto robot = DummyRobot{};
  const auto robot_cb = [&robot](const grape::robot::loco::Command& cmd) { robot.move(cmd); };
  auto loco_service = grape::robot::loco::Service(ROBOT_NAME, robot_cb);

  // That's it. Run the teleop client example to interact
  std::println("[{}] Locomotion service started. Press ctrl-c to exit.", ROBOT_NAME);
  grape::app::waitForExit();

  return EXIT_SUCCESS;
}
