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

struct State {
  bool is_service_detected{ false };
  bool is_client_active{ false };
  grape::WallClock::Duration latency{};
  std::string last_error_msg;
};

//-------------------------------------------------------------------------------------------------
auto toStatusUi(const std::string& robot_name, bool en, State state)
    -> std::pair<ftxui::Element, ftxui::Element> {
  const auto service_status =
      state.is_service_detected ?
          ftxui::text(" [Reachable    ]") | ftxui::color(ftxui::Color::Green) :
          ftxui::text(" [Not Reachable]") | ftxui::color(ftxui::Color::YellowLight);

  // service_detected | teleop_enabled | client_active | reported state
  // -----------------|----------------|---------------|---------------
  // 0                | x              | x             | unknown
  // 1                | 0              | 0             | inactive
  // 1                | 0              | 1             | canceling
  // 1                | 1              | 0             | requesting
  // 1                | 1              | 1             | active

  const auto client_status = [&] {
    if (!state.is_service_detected) {
      return ftxui::text(" [Control: Unknown   ]") | ftxui::color(ftxui::Color::YellowLight);
    }
    if (!en && !state.is_client_active) {
      return ftxui::text(" [Control: Inactive  ]") | ftxui::color(ftxui::Color::GrayLight);
    }
    if (!en && state.is_client_active) {
      return ftxui::text(" [Control: Canceling ]") | ftxui::color(ftxui::Color::YellowLight);
    }
    if (en && !state.is_client_active) {
      return ftxui::text(" [Control: Requesting]") | ftxui::color(ftxui::Color::YellowLight);
    }
    if (en && state.is_client_active) {
      return ftxui::text(" [Control: Active    ]") | ftxui::color(ftxui::Color::Green);
    }
    return ftxui::text(" [Control: ??        ]") | ftxui::color(ftxui::Color::Red);
  }();

  const auto latency_text = ftxui::text(std::format(" [Latency: {:<10}]", state.latency));
  static auto error_msg = ftxui::Element{ ftxui::text("No Errors") };
  error_msg = state.last_error_msg.empty() ?
                  error_msg :
                  ftxui::text(std::format("Last Error: {}", state.last_error_msg)) |
                      ftxui::color(ftxui::Color::YellowLight);
  return { ftxui::hbox({ ftxui::text("Robot: "), ftxui::text(robot_name) | ftxui::bold,
                         service_status, client_status, latency_text }),
           error_msg };
}

//-------------------------------------------------------------------------------------------------
auto onTeleopStatus(const grape::locomotion::TeleopClient::Status& teleop_status) -> State {
  static auto state = State{};
  struct Visitor {
    using TeleopClient = grape::locomotion::TeleopClient;
    void operator()(const TeleopClient::ServiceStatus& st) {
      state.is_service_detected = st.is_detected;
    }
    void operator()(const TeleopClient::ClientStatus& st) {
      state.is_client_active = st.is_client_active;
      state.latency = st.command_latency;
    }
    void operator()(const TeleopClient::Error& st) {
      const auto now = grape::WallClock::now();
      state.last_error_msg = std::format("[{}]: {}", now, st.message);
    }
  };
  std::visit(Visitor(), teleop_status);
  return state;
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

    const auto ipc_config = grape::ipc::Config{ .scope = grape::ipc::Config::Scope::Network };
    grape::ipc::init(ipc_config);

    auto state = State{};
    auto teleoperator = grape::locomotion::TeleopClient(
        robot_name, [&state](const auto& status) { state = onTeleopStatus(status); });
    auto screen = ftxui::ScreenInteractive::Fullscreen();

    auto cmd_error = false;
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
      const auto [status_ui, error_ui] = toStatusUi(robot_name, teleop_enable, state);
      const auto cmd_ui =
          ftxui::hbox({ ftxui::text(toString(move_cmd)) |
                        ftxui::color(cmd_error ? ftxui::Color::Red : ftxui::Color::White) });
      return ftxui::vbox({ ftxui::text("Keyboard Teleop") | ftxui::bold | ftxui::center,  //
                           ftxui::separator(),                                            //
                           status_ui,                                                     //
                           ftxui::separator(),                                            //
                           ftxui::text("Controls:") | ftxui::bold,                        //
                           ftxui::text("  SPACE - Enable/Disable Teleop"),                //
                           ftxui::text("  +/-   - Speed step up/down"),                   //
                           ftxui::text("  ↑/↓   - Forward/Backward"),                     //
                           ftxui::text("  ←/→   - Lateral Left/Right"),                   //
                           ftxui::text("  </>   - Rotate Left/Right"),
                           ftxui::text("  ESC   - Exit"),          //
                           ftxui::separator(),                     //
                           ftxui::text("Command:") | ftxui::bold,  //
                           ftxui::hbox({ ftxui::text("Step Speed: "),
                                         ftxui::text(std::format("{}", speed_scale)) }),  //
                           cmd_ui,                                                        //
                           ftxui::separator(),                                            //
                           error_ui }) |
             ftxui::border;
    });

    static constexpr auto CONTROL_PERIOD = std::chrono::milliseconds(100);
    auto loop = ftxui::Loop(&screen, main_component | key_handler);
    while (not loop.HasQuitted() and grape::ipc::ok()) {
      const auto start_update_ts = std::chrono::steady_clock::now();
      loop.RunOnce();
      if (teleop_enable) {
        cmd_error = !teleoperator.send(move_cmd);
      }
      screen.PostEvent(ftxui::Event::Custom);
      move_cmd = {};

      auto elapsed = std::chrono::steady_clock::now() - start_update_ts;
      auto sleep_time = CONTROL_PERIOD - elapsed;
      if (sleep_time > std::chrono::milliseconds(0)) {
        std::this_thread::sleep_for(sleep_time);
      }
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
