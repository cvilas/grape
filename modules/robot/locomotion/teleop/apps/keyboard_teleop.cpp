//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <numbers>
#include <print>
#include <thread>

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "grape/conio/program_options.h"
#include "grape/ipc/session.h"
#include "grape/locomotion/teleop_client.h"

namespace {
//-------------------------------------------------------------------------------------------------
auto status(bool en, const grape::locomotion::TeleopClient::Status& st) -> ftxui::Element {
  if (not st.is_service_detected) {
    return ftxui::text("Service Unreachable") | ftxui::color(ftxui::Color::Red);
  }
  if ((not en) and (not st.is_client_active)) {
    return ftxui::text("Inactive") | ftxui::color(ftxui::Color::GrayLight);
  }
  if ((not en) and st.is_client_active) {
    return ftxui::text("Canceling") | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
  }
  if (en and (not st.is_client_active)) {
    return ftxui::text("Requesting") | ftxui::color(ftxui::Color::Yellow) | ftxui::bold;
  }
  if (en and st.is_client_active) {
    return ftxui::text("Active") | ftxui::color(ftxui::Color::Green) | ftxui::bold;
  }
  return ftxui::text("Unknown") | ftxui::color(ftxui::Color::Red) | ftxui::bold;
}
}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto args = grape::conio::ProgramDescription("Keyboard teleop console")
                          .declareOption<std::string>("robot", "Robot name", "dummy_robot")
                          .parse(argc, argv);
    const auto robot_name = args.getOption<std::string>("robot");
    static constexpr auto MAX_LINEAR_SPEED = 1.F;
    static constexpr auto MAX_TURN_SPEED = 30.F * static_cast<float>(std::numbers::pi) / 180.F;
    static constexpr auto SPEED_STEP = 0.1F;

    auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
    grape::ipc::init(std::move(ipc_config));

    auto teleop_status = grape::locomotion::TeleopClient::Status{};
    const auto on_teleop_status = [&teleop_status](const auto& status) { teleop_status = status; };
    auto teleoperator = grape::locomotion::TeleopClient(robot_name, on_teleop_status);
    auto screen = ftxui::ScreenInteractive::Fullscreen();

    auto teleop_enable = false;
    auto move_cmd = grape::locomotion::Move3DCmd{};
    auto speed_scale = 0.F;

    auto key_handler = ftxui::CatchEvent([&](const ftxui::Event& event) -> bool {
      speed_scale = std::min(std::max(speed_scale, 0.F), 1.F);
      const auto linear_speed = speed_scale * MAX_LINEAR_SPEED;
      const auto turn_speed = speed_scale * MAX_TURN_SPEED;
      if (event == ftxui::Event::Escape) {
        screen.ExitLoopClosure()();
        return true;
      }

      if ((event == ftxui::Event::Character('+')) or (event == ftxui::Event::Character('='))) {
        speed_scale += SPEED_STEP;
        return true;
      }

      if ((event == ftxui::Event::Character('-')) or (event == ftxui::Event::Character('_'))) {
        speed_scale -= SPEED_STEP;
        return true;
      }

      if (event == ftxui::Event::Character(' ')) {
        teleop_enable = !teleop_enable;
        return true;
      }

      if (event == ftxui::Event::ArrowUp) {
        move_cmd.forward_speed = linear_speed;
        return true;
      }

      if (event == ftxui::Event::ArrowDown) {
        move_cmd.forward_speed = -linear_speed;
        return true;
      }

      if (event == ftxui::Event::ArrowLeft) {
        move_cmd.lateral_speed = linear_speed;
        return true;
      }

      if (event == ftxui::Event::ArrowRight) {
        move_cmd.lateral_speed = -linear_speed;
        return true;
      }

      if ((event == ftxui::Event::Character('<')) or (event == ftxui::Event::Character(','))) {
        move_cmd.turn_speed = turn_speed;
        return true;
      }

      if ((event == ftxui::Event::Character('>')) or (event == ftxui::Event::Character('.'))) {
        move_cmd.turn_speed = -turn_speed;
        return true;
      }

      return false;
    });

    // Main UI renderer
    auto main_component = ftxui::Renderer([&] {
      return ftxui::vbox({
                 ftxui::text("Keyboard Teleop") | ftxui::bold | ftxui::center,
                 ftxui::separator(),
                 ftxui::hbox({
                     ftxui::text("Robot: "),
                     ftxui::text(robot_name) | ftxui::color(ftxui::Color::Cyan) | ftxui::bold |
                         ftxui::flex,
                     ftxui::text("Status: "),
                     status(teleop_enable, teleop_status) | ftxui::flex,
                     ftxui::text("Command Latency: "),
                     ftxui::text(std::format("{:<10}", teleop_status.command_latency)),
                 }),
                 ftxui::separator(),
                 ftxui::text("Controls:") | ftxui::bold,
                 ftxui::text("  SPACE - Enable/Disable Teleop"),
                 ftxui::text("  +/-   - Speed step up/down"),
                 ftxui::text("  ↑/↓   - Forward/Backward"),
                 ftxui::text("  ←/→   - Lateral Left/Right"),
                 ftxui::text("  </>   - Rotate Left/Right"),
                 ftxui::text("  ESC   - Exit"),
                 ftxui::separator(),
                 ftxui::text("Command:") | ftxui::bold,
                 ftxui::hbox(
                     { ftxui::text("Step Speed: "), ftxui::text(std::format("{}", speed_scale)) }),
                 ftxui::hbox({ ftxui::text(std::format("{}", toString(move_cmd))) }),
                 ftxui::separator(),
             }) |
             ftxui::border;
    });

    static constexpr auto CONTROL_PERIOD = std::chrono::milliseconds(100);
    auto loop = ftxui::Loop(&screen, main_component | key_handler);
    while (not loop.HasQuitted() and grape::ipc::ok()) {
      loop.RunOnce();
      if (teleop_enable) {
        teleoperator.send(move_cmd);
      }
      move_cmd = {};
      screen.PostEvent(ftxui::Event::Custom);
      std::this_thread::sleep_for(CONTROL_PERIOD);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
