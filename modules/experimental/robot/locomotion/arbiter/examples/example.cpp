//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <print>

#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"
#include "grape/locomotion/arbiter.h"

namespace {

//=================================================================================================
class DummyRobot {
public:
  explicit DummyRobot(std::string robot_name) : name_(std::move(robot_name)) {
  }
  void move(const grape::locomotion::Command& cmd) {
    std::visit([this](const auto& val) { std::println("[{}] {}", name_, toString(val)); }, cmd);
  }

private:
  std::string name_;
};

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//=================================================================================================
// Creates a dummy robot locomotion service utilising the command arbiter.
// Paired with teleop_example to demonstrate arbiter behaviour
auto main(int argc, const char* argv[]) -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    const auto args = grape::conio::ProgramDescription("Dummy robot locomotion service example")
                          .declareOption<std::string>("robot", "Robot name", "dummy_robot")
                          .parse(argc, argv);

    const auto robot_name = args.getOption<std::string>("robot");

    const auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
    grape::ipc::init(ipc_config);

    // Create a dummy robot platform and hook command arbitrator to it
    auto robot = DummyRobot(robot_name);
    const auto robot_cb = [&robot](const grape::locomotion::Command& cmd) { robot.move(cmd); };
    auto loco_service = grape::locomotion::Arbiter(robot_name, robot_cb);

    // That's it. Run the teleop client example to interact
    std::println("[{}] Locomotion service started. Press ctrl-c to exit.", robot_name);
    s_exit.wait(false);
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
