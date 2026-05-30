//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numbers>
#include <random>
#include <thread>

#include "grape/exception.h"
#include "grape/plot/window.h"
#include "grape/wall_clock.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates how to extend plotting support to custom data types
//-------------------------------------------------------------------------------------------------

namespace {
//-------------------------------------------------------------------------------------------------
// Custom data structure: battery status
//
struct BatteryStatus {
  grape::WallClock::TimePoint timestamp;
  std::uint8_t state_of_charge{};
  double current{};
  double terminal_voltage{};
};

//-------------------------------------------------------------------------------------------------
// Free function extends plotting support for custom data structure
//
void plot(const BatteryStatus& data, grape::plot::Window& plot) {
  static constexpr auto LABEL_FIELD_0 = "SoC (%)";
  static constexpr auto LABEL_FIELD_1 = "Current (A)";
  static constexpr auto LABEL_FIELD_2 = "Voltage (V)";
  static auto first_ts = grape::WallClock::TimePoint{};
  if (first_ts == grape::WallClock::TimePoint{}) {
    first_ts = data.timestamp;
  }
  const auto dt = std::chrono::duration<double>(data.timestamp - first_ts).count();

  const auto get_trace = [&](const char* label) -> grape::plot::Trace& {
    if (auto* tr = plot.trace(label); tr != nullptr) {
      return *tr;
    }
    return plot.createTrace(label);
  };

  get_trace(LABEL_FIELD_0).addData({ .x = dt, .y = static_cast<double>(data.state_of_charge) });
  get_trace(LABEL_FIELD_1).addData({ .x = dt, .y = data.current });
  get_trace(LABEL_FIELD_2).addData({ .x = dt, .y = data.terminal_voltage });
}

//-------------------------------------------------------------------------------------------------
// Emulates battery using a simplified circuit model
//
auto emulate() -> BatteryStatus {
  static constexpr auto CAPACITY = 10.0;        // battery capacity (Ah)
  static constexpr auto OCV_FULL = 25.2;        // open circuit voltage at full (V)
  static constexpr auto OCV_EMPTY = 20.0;       // open circuit voltage at empty (V)
  static constexpr auto INTERNAL_RESIS = 0.05;  // internal resistance (Ohm)
  static constexpr auto NOISE_CURRENT = 0.02;   // current noise amplitude (A)
  static constexpr auto CONSTANT_CURRENT = 5.;

  const auto ts = grape::WallClock::now();
  static auto last_ts = ts;
  const auto dt = std::chrono::duration<double>(ts - last_ts).count();
  last_ts = ts;

  static thread_local std::mt19937 rng{ std::random_device{}() };
  static thread_local std::uniform_real_distribution<double> dist(0, NOISE_CURRENT);

  static auto soc = 1.0;  // State of charge [0, 1]
  const auto current = (soc <= 0.0) ? 0.0 : CONSTANT_CURRENT + dist(rng);

  static constexpr auto SECS_PER_HR = 3600.0;
  soc = std::clamp(soc - ((current * dt) / (CAPACITY * SECS_PER_HR)), 0.0, 1.0);
  const auto ocv = OCV_EMPTY + ((OCV_FULL - OCV_EMPTY) * soc);
  const auto terminal_volts = std::max(0.0, ocv - (current * INTERNAL_RESIS));

  return {
    .timestamp = ts,
    .state_of_charge = static_cast<std::uint8_t>(soc * 100.0),
    .current = current,
    .terminal_voltage = terminal_volts,
  };
}
}  // namespace

//=================================================================================================
//
auto main() -> int {
  try {
    static constexpr auto WINDOW_WIDTH = 960;
    static constexpr auto WINDOW_HEIGHT = 600;

    auto plot_window = grape::plot::Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Battery status");
    plot_window.setAxisText(grape::plot::AxisId::AxisX, "Time (s)");
    plot_window.enableLegend(true);

    // monitoring thread generates data
    const auto data_thread = std::jthread([&](const std::stop_token& stoken) {
      static constexpr auto UPDATE_INTERVAL = std::chrono::milliseconds(1);
      while (not stoken.stop_requested() and plot_window.isOpen()) {
        const auto status = emulate();
        plot(status, plot_window);
        std::this_thread::sleep_for(UPDATE_INTERVAL);
      }
    });

    // main thread redraws plot
    while (plot_window.processEvents()) {
      /* runs at display refresh rate */
      plot_window.render();
    }

  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
