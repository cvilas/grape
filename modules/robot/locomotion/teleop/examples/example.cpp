//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "grape/conio/conio.h"
#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"
#include "grape/locomotion/teleop_client.h"

//=================================================================================================
// Example demonstrates teleop controller API.
// Paired with robot_loco_service_example (in service module) to demonstrate behaviour
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto args_opt = grape::conio::ProgramDescription("Robot teleop client example")
                              .declareOption<std::string>("robot", "Robot name", "dummy_robot")
                              .parse(argc, argv);
    if (not args_opt.has_value()) {
      throw std::runtime_error(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto robot_name = args.getOptionOrThrow<std::string>("robot");

    auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
    grape::ipc::init(std::move(ipc_config));

    const auto on_teleop_status = [](const auto& status) -> void {
      std::println("Teleop status: is_service_detected={}, is_client_active={}, command_latency={}",
                   status.is_service_detected, status.is_client_active, status.command_latency);
    };

    auto teleoperator = grape::locomotion::TeleopClient(robot_name, on_teleop_status);
    auto enable = false;
    static constexpr auto CONTROL_PERIOD = std::chrono::milliseconds(100);
    while (grape::ipc::ok()) {
      if (grape::conio::kbhit()) {
        const auto key = grape::conio::getch();
        if (key == 't') {
          enable = !enable;
          std::println("Teleop {}", (enable ? "requested" : "request cancelled"));
        }
      }
      if (enable) {
        teleoperator.send(grape::locomotion::KeepAliveCmd{});  // No-op; keep teleop alive
      }
      std::this_thread::sleep_for(CONTROL_PERIOD);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
