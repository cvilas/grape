//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
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

namespace {

//-------------------------------------------------------------------------------------------------
// signal handler
std::atomic_bool s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
void onSignal(int /*signum*/) {
  s_exit = true;
}

//-------------------------------------------------------------------------------------------------
// prints record to console
template <grape::probe::NumericType T>
void print(std::string_view name, std::span<const std::byte> data) {
  const auto count = data.size_bytes() / sizeof(T);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto value = std::span<const T>(reinterpret_cast<const T*>(data.data()), count);
  std::print("{}=[ ", name);
  for (const auto& v : value) {
    std::print("{} ", v);
  }
  std::println("]");
}

//-------------------------------------------------------------------------------------------------
// deserialises record
void sink(const std::vector<grape::probe::Signal>& signals, std::span<const std::byte> data) {
  auto offset = 0UL;
  for (const auto& s : signals) {
    const auto size_bytes = length(s.type) * s.num_elements;
    using TID = grape::probe::TypeId;
    switch (s.type) {
        // clang-format off
      case TID::Int8: print<std::int8_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Uint8: print<std::uint8_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Int16: print<std::int16_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Uint16: print<std::uint16_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Int32: print<std::int32_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Uint32: print<std::uint32_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Int64: print<std::int64_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Uint64: print<std::uint64_t>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Float32: print<float>(s.name.str(), data.subspan(offset, size_bytes)); break;
      case TID::Float64: print<double>(s.name.str(), data.subspan(offset, size_bytes)); break;
        // clang-format on
    }
    offset += size_bytes;
  }
  std::println("");
}

}  // namespace

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;

  (void)signal(SIGINT, onSignal);
  (void)signal(SIGTERM, onSignal);

  try {
    double timestamp = 0.;
    double amplitude = 1.;
    double frequency = 1.;
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
    auto probe = grape::probe::Controller(std::move(pin_config), BUFFER_CONFIG, sink);

    // Process function: In real use-cases, this could be executing in a privileged
    // time sensitive context in a separate thread. Here, we just periodically update variables
    const auto process = [&probe, &timestamp, &amplitude, &frequency, &waveforms]() {
      static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(100);
      const auto ts_start = std::chrono::high_resolution_clock::now();
      while (not s_exit) {
        const auto ts = std::chrono::high_resolution_clock::now();

        //----- process step update begin ------
        const auto dt = ts - ts_start;
        timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
        waveforms.at(0) = amplitude * std::cos(2 * std::numbers::pi * frequency * timestamp);
        waveforms.at(1) = amplitude * std::sin(2 * std::numbers::pi * frequency * timestamp);
        //----- process step update end ------

        // Grab snapshot of all registered variables
        if (not probe.snap()) {
          std::println("Could not snap");
        }

        // update control variables for next step update
        probe.sync();

        std::this_thread::sleep_until(ts + LOOP_PERIOD);
      }
    };
    //-------------------------------------------------------------------------------

    // Monitor function: In real use-cases, this would be executing in non realtime context. Here,
    // we periodically receive batched updates and show how to queue a control variable update
    const auto monitor = [&probe]() {
      static constexpr auto LOOP_PERIOD = std::chrono::milliseconds(1000);
      double desired_amplitude = {};
      while (not s_exit) {
        const auto ts = std::chrono::high_resolution_clock::now();

        // Flush snapshots to sink
        probe.flush();

        // Queue update for a control parameter
        static constexpr auto AMPLITUDE_DELTA = 0.1;
        desired_amplitude += AMPLITUDE_DELTA;
        if (not probe.qset("amplitude", desired_amplitude)) {
          std::println("Error setting control variable");
        }

        std::this_thread::sleep_until(ts + LOOP_PERIOD);
      }
    };
    //-------------------------------------------------------------------------------

    auto th = std::thread(process);  //!< run process in a separate thread
    monitor();                       //!< monitor the process in main thread
    if (th.joinable()) {
      th.join();
    }
    return EXIT_SUCCESS;
  } catch (const grape::probe::ControllerException&) {
    grape::probe::ControllerException::consume();
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
