//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <print>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"
#include "grape/robot/loco/teleop_client.h"

//=================================================================================================
// FTXUI-based keyboard teleop application
// Arrow keys: Forward/backward and lateral motion
// < and > keys: Rotation left/right
// Space: Enable/disable teleop
// ESC: Exit
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto args_opt = grape::conio::ProgramDescription("Robot keyboard teleop with FTXUI")
                              .declareOption<std::string>("robot", "Robot name", "dummy_robot")
                              .parse(argc, argv);
    if (not args_opt.has_value()) {
      throw std::runtime_error(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto robot_name = args.getOptionOrThrow<std::string>("robot");
    const auto max_speed = 1.F;
    const auto max_turn_speed = 1.F;

    auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
    grape::ipc::init(std::move(ipc_config));

    // Teleop state
    auto teleop_enabled = false;
    auto move_cmd = grape::robot::loco::Move3DCmd{};
    auto teleop_status = grape::robot::loco::TeleopClient::Status{};

    const auto on_teleop_status = [&teleop_status](const auto& status) { teleop_status = status; };

    auto teleoperator = grape::robot::loco::TeleopClient(robot_name, on_teleop_status);

    // UI Components
    auto screen = ftxui::ScreenInteractive::Fullscreen();

    // Key handler for custom controls
    auto key_handler = ftxui::CatchEvent([&](const ftxui::Event& event) -> bool {
      // Reset command each event
      move_cmd = {};

      if (event == ftxui::Event::Escape) {
        screen.ExitLoopClosure()();
        return true;
      }

      if (event == ftxui::Event::Character(' ')) {
        teleop_enabled = !teleop_enabled;
        return true;
      }

      if (event == ftxui::Event::ArrowUp) {
        move_cmd.forward_speed = max_speed;
        return true;
      }
      if (event == ftxui::Event::ArrowDown) {
        move_cmd.forward_speed = -max_speed;
        return true;
      }
      if (event == ftxui::Event::ArrowLeft) {
        move_cmd.lateral_speed = max_speed;
        return true;
      }
      if (event == ftxui::Event::ArrowRight) {
        move_cmd.lateral_speed = -max_speed;
        return true;
      }

      if (event == ftxui::Event::Character('<')) {
        move_cmd.turn_speed = max_turn_speed;
        return true;
      }
      if (event == ftxui::Event::Character('>')) {
        move_cmd.turn_speed = -max_turn_speed;
        return true;
      }

      return false;
    });

    // Main UI renderer
    auto main_component = ftxui::Renderer([&] {
      auto status_color = teleop_enabled ? ftxui::Color::Green : ftxui::Color::Red;
      const auto* const status_text = teleop_enabled ? "ENABLED" : "DISABLED";

      return ftxui::vbox({
                 ftxui::text("Keyboard Teleop") | ftxui::bold | ftxui::center,
                 ftxui::separator(),
                 ftxui::hbox({ftxui::text("Robot: "), ftxui::text(robot_name) | ftxui::color(ftxui::Color::Cyan),}),
                 ftxui::hbox({ftxui::text("Teleop Status: "), ftxui::text(status_text) | ftxui::color(status_color) | ftxui::bold,}),
                 ftxui::hbox({ftxui::text("Service Active: "), ftxui::text(teleop_status.is_active ? "YES" : "NO") | ftxui::color(teleop_status.is_active ? ftxui::Color::Green : ftxui::Color::Red),}),
                 ftxui::hbox({ftxui::text("Command Latency: "), ftxui::text(std::format("{}", teleop_status.command_latency)),}),
                 ftxui::separator(),
                 ftxui::text("Controls:") | ftxui::bold,
                 ftxui::text("  SPACE - Enable/Disable Teleop"),
                 ftxui::text("  ↑/↓   - Forward/Backward"),
                 ftxui::text("  ←/→   - Lateral Left/Right"),
                 ftxui::text("  < />  - Rotate Left/Right"),
                 ftxui::text("  ESC   - Exit"),
                 ftxui::separator(),
                 ftxui::text("Current Command:") | ftxui::bold,
                 ftxui::hbox({ftxui::text("  Forward: "), ftxui::text(std::format("{:+.2f}", move_cmd.forward_speed)) | ftxui::color(move_cmd.forward_speed != 0.0F ? ftxui::Color::Yellow : ftxui::Color::White),}),
                 ftxui::hbox({ftxui::text("  Lateral: "), ftxui::text(std::format("{:+.2f}", move_cmd.lateral_speed)) | ftxui::color(move_cmd.lateral_speed != 0.0F ? ftxui::Color::Yellow : ftxui::Color::White),}),
                 ftxui::hbox({ftxui::text("  Turn:    "), ftxui::text(std::format("{:+.2f}", move_cmd.turn_speed)) | ftxui::color(move_cmd.turn_speed != 0.0F ? ftxui::Color::Yellow : ftxui::Color::White),}),
                 ftxui::separator(),
                 ftxui::text("Press SPACE to enable teleop, then use arrow keys and < > to control the robot") | ftxui::dim | ftxui::center,
             }) | ftxui::border;
    });

    // Combine key handler with main component
    auto app = main_component | key_handler;

    // Command sending loop in separate thread
    std::atomic<bool> running{ true };
    auto command_thread = std::thread([&]() {
      static constexpr auto CONTROL_PERIOD = std::chrono::milliseconds(100);

      while (running && grape::ipc::ok()) {
        if (teleop_enabled) {
          teleoperator.send(move_cmd);
        }
        std::this_thread::sleep_for(CONTROL_PERIOD);
      }
    });

    // Run the UI
    screen.Loop(app);

    // Cleanup
    running = false;
    if (command_thread.joinable()) {
      command_thread.join();
    }

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
