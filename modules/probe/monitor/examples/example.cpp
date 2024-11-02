//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <numbers>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/probe/controller.h"
#include "grape/probe/monitor.h"

//=================================================================================================
auto main() -> int {
  std::atomic_bool exit_flag = false;
  try {
    double timestamp = 0.;
    double amplitude = 1.;  //!< NOLINT(cppcoreguidelines-avoid-magic-numbers)
    double frequency = .5;  //!< NOLINT(cppcoreguidelines-avoid-magic-numbers)
    std::vector<double> waveforms = { 0., 0. };

    // configure the pins
    using Role = grape::probe::Signal::Role;
    auto pin_config = grape::probe::PinConfig()
                          .pin("timestamp", timestamp, Role::Timestamp)  //
                          .pin("amplitude", amplitude, Role::Control)    //
                          .pin("frequency", frequency, Role::Control)    //
                          .pin("waveforms", std::span<const double>{ waveforms }, Role::Watch);

    // configure the capture buffers
    static constexpr auto BUFFER_CONFIG = grape::probe::BufferConfig{
      .snap_buffer_capacity = 100U,  //
      .sync_buffer_capacity = 10U    //
    };

    // create the probe controller
    auto monitor = grape::probe::Monitor();
    auto data_sink = [&monitor](const std::vector<grape::probe::Signal>& signals,
                                std::span<const std::byte> data) { monitor.recv(signals, data); };
    auto probe = grape::probe::Controller(std::move(pin_config), BUFFER_CONFIG, data_sink);
    monitor.setSender([&probe](const std::string& name, std::span<const std::byte> data) {
      const auto ret = probe.qset(name, data);
      if (ret != grape::probe::Controller::Error::None) {
        grape::panic<grape::Exception>(std::format("{}: {}", name, toString(ret)));
      }
    });

    // Process function: In real use-cases, this could be executing in a privileged
    // time sensitive context in a separate thread. Here, we just periodically update variables
    const auto process_loop = [&exit_flag, &probe, &timestamp, &amplitude, &frequency,
                               &waveforms]() {
      try {
        static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(10);
        const auto ts_start = std::chrono::high_resolution_clock::now();
        while (not exit_flag) {
          const auto ts = std::chrono::high_resolution_clock::now();

          //----- process step update begin ------
          const auto dt = ts - ts_start;
          timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
          waveforms.at(0) = amplitude * std::cos(2 * std::numbers::pi * frequency * timestamp);
          waveforms.at(1) = amplitude * std::sin(2 * std::numbers::pi * frequency * timestamp);
          //----- process step update end ------

          // Grab snapshot of all registered variables
          if (probe.snap() != grape::probe::Controller::Error::None) {
            std::println("Could not snap");
          }

          // update control variables for next step update
          probe.sync();

          std::this_thread::sleep_until(ts + LOOP_PERIOD);
        }
      } catch (...) {
        grape::Exception::print();
        exit_flag = true;
      }
    };

    // Monitor function: In real use-cases, this would be executing in non realtime context. Here,
    // we periodically receive batched updates and show how to queue a control variable update
    const auto monitor_loop = [&exit_flag, &probe]() {
      try {
        static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(100);
        while (not exit_flag) {
          const auto ts = std::chrono::high_resolution_clock::now();
          // Flush snapshots to sink
          probe.flush();
          std::this_thread::sleep_until(ts + LOOP_PERIOD);
        }
      } catch (...) {
        grape::Exception::print();
        exit_flag = true;
      }
    };

    auto process_thread = std::thread(process_loop);
    auto monitor_thread = std::thread(monitor_loop);
    monitor.run();
    exit_flag = true;
    if (process_thread.joinable()) {
      process_thread.join();
    }
    if (monitor_thread.joinable()) {
      monitor_thread.join();
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
