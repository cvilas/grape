//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <cmath>
#include <numbers>
#include <thread>

#include "grape/exception.h"
#include "grape/plot/window.h"

//=================================================================================================
/// Demonstrates realtime plotting api
///
auto main() -> int {
  try {
    static constexpr auto WINDOW_WIDTH = 960;
    static constexpr auto WINDOW_HEIGHT = 600;

    auto plot = grape::plot::Window(WINDOW_WIDTH, WINDOW_HEIGHT, "Plot demo");
    plot.setAxisText(grape::plot::AxisId::AxisX, "Time (s)");
    plot.setAxisText(grape::plot::AxisId::AxisY, "Amplitude");
    plot.enableLegend(true);

    auto& sine = plot.trace("Sine");

    auto& cosine = plot.trace("Cosine");
    cosine.setLineStyle(grape::plot::LineStyle::Step);

    auto& beat = plot.trace("Beat");
    beat.setPointStyle(grape::plot::PointStyle::Dot);
    beat.setLineStyle(grape::plot::LineStyle::Line);

    // data generation thread
    const auto data_thread = std::jthread([&](const std::stop_token& stoken) {
      using clock = std::chrono::steady_clock;
      static constexpr auto UPDATE_INTERVAL = std::chrono::milliseconds(10);
      auto ticks = std::uint64_t{};
      const auto t0 = clock::now();
      while (not stoken.stop_requested() and plot.isOpen()) {
        const auto tn = std::chrono::duration<double>(clock::now() - t0).count();

        // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
        using std::numbers::pi;
        sine.addData({
            .x = tn,
            .y = std::sin(2.0 * pi * 0.5 * tn),
        });
        cosine.addData({
            .x = tn,
            .y = std::cos(2.0 * pi * 0.3 * tn) * 0.7,
        });
        beat.addData({
            .x = tn,
            .y = (std::sin(2.0 * pi * 1.0 * tn) + std::sin(2.0 * pi * 1.1 * tn)) * 0.4,
        });
        // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

        // Skip ahead to the next future tick if we've fallen behind (e.g. after SIGTSTP/SIGCONT).
        // Without this, sleep_until returns immediately for every accumulated past target,
        // causing the thread to spin at CPU speed and contend on the FIFO's atomic counter,
        // which stalls the render thread.
        ++ticks;
        const auto next_tick = t0 + ticks * UPDATE_INTERVAL;
        if (clock::now() >= next_tick) {
          ticks = static_cast<std::uint64_t>((clock::now() - t0) / UPDATE_INTERVAL) + 1U;
        }
        std::this_thread::sleep_until(t0 + ticks * UPDATE_INTERVAL);
      }
    });

    // main loop updates plot
    while (plot.processEvents()) {
      /* runs at display refresh rate */
      plot.render();
    }

  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
