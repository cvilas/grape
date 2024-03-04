//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "grape/probe/controller.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp)

//-------------------------------------------------------------------------------------------------
TEST_CASE("[Controller is configured correctly]", "[controller]") {
  const double timestamp = 0.;
  const double amplitude = 1.;
  const double frequency = 1.;
  std::vector<double> waveforms = { 0., 0. };

  // configure the pins
  using Role = grape::probe::Signal::Role;
  auto pins = grape::probe::PinConfig()
                  .pin("timestamp", timestamp, Role::Watch)    //
                  .pin("amplitude", amplitude, Role::Control)  //
                  .pin("frequency", frequency, Role::Control)  //
                  .pin("waveforms", std::span<const double>{ waveforms }, Role::Watch);

  using Signal = grape::probe::Signal;
  const auto controllables_start = pins.sort();
  const auto& signals = pins.signals();

  // check pins are sorted and controllables are at the back
  const auto sort_predicate = [](const Signal& a, const Signal& b) {
    return ((a.role == b.role) ? (a.address < b.address) : (a.role == Signal::Role::Watch));
  };

  CHECK(std::ranges::is_sorted(signals, sort_predicate));
  CHECK(std::all_of(controllables_start, signals.end(),
                    [r = Role::Control](const auto& s) { return s.role == r; }));
  CHECK(std::all_of(signals.begin(), controllables_start,
                    [r = Role::Watch](const auto& s) { return s.role == r; }));

  // configure the capture buffers
  static constexpr auto BUFFER_CONFIG = grape::probe::BufferConfig{
    .snap_buffer_capacity = 3UZ,  //
    .sync_buffer_capacity = 2UZ   //
  };

  // create the probe controller
  auto num_records = 0UZ;
  const auto sink = [&num_records](const std::vector<Signal>&, std::span<const std::byte>) {
    num_records++;
  };

  auto probe = grape::probe::Controller(std::move(pins), BUFFER_CONFIG, sink);

  // Check we can snap expected number of times
  CHECK(probe.snap().has_value());
  CHECK(probe.snap().has_value());
  CHECK(probe.snap().has_value());
  CHECK_FALSE(probe.snap().has_value());

  // Check subsequent flush yields expected number of records
  probe.flush();
  CHECK(num_records == BUFFER_CONFIG.snap_buffer_capacity);

  // Check we can qset expected number of times
  static constexpr auto AMPLITUDE_DELTA = 0.1;
  auto amplitude_setting = 0.0;
  CHECK(probe.qset("amplitude", amplitude_setting).has_value());
  amplitude_setting += AMPLITUDE_DELTA;
  CHECK(probe.qset("amplitude", amplitude_setting).has_value());
  CHECK_FALSE(probe.qset("amplitude", amplitude_setting + AMPLITUDE_DELTA).has_value());

  // Check after sync, the control variable is set to latest value from qset
  probe.sync();
  constexpr auto EPSILON = 0.0001;
  CHECK_THAT(amplitude, Catch::Matchers::WithinRel(amplitude_setting, EPSILON));
}

// NOLINTEND(cert-err58-cpp)

}  // namespace
