//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <algorithm>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "grape/probe/controller.h"

namespace {

//-------------------------------------------------------------------------------------------------
TEST_CASE("[Controller is configured correctly]", "[controller]") {
  const double timestamp = 0.;
  const double amplitude = 1.;
  const double frequency = 1.;
  std::vector<double> waveforms = { 0., 0. };

  // configure the pins
  using Role = grape::probe::Signal::Role;
  auto pins = grape::probe::PinConfig()
                  .pin("timestamp", timestamp, Role::Timestamp)  //
                  .pin("amplitude", amplitude, Role::Control)    //
                  .pin("frequency", frequency, Role::Control)    //
                  .pin("waveforms", std::span<const double>{ waveforms }, Role::Watch);

  using Signal = grape::probe::Signal;
  const auto controllables = pins.sort();
  CHECK(controllables.size() == 2);
  const auto& signals = pins.signals();

  // check pins are sorted by address and role
  const auto sort_predicate = [](const Signal& sig_a, const Signal& sig_b) noexcept -> bool {
    if (sig_a.role == sig_b.role) {
      return (sig_a.address < sig_b.address);
    }
    return (sig_a.role < sig_b.role);
  };

  CHECK(std::ranges::is_sorted(signals, sort_predicate));
  CHECK(
      std::ranges::all_of(controllables, [role = Role::Control](const auto& sig) noexcept -> auto {
        return sig.role == role;
      }));

  // configure the capture buffers
  static constexpr auto BUFFER_CONFIG = grape::probe::BufferConfig{
    .snap_buffer_capacity = 3UZ,  //
    .sync_buffer_capacity = 2UZ   //
  };

  // create the probe controller
  auto num_records = 0UZ;
  const auto sink = [&num_records](const std::vector<Signal>&, std::span<const std::byte>) -> void {
    num_records++;
  };

  auto probe = grape::probe::Controller(std::move(pins), BUFFER_CONFIG, sink);

  // Check we can snap expected number of times
  CHECK(probe.snap() == grape::probe::Controller::Error::None);
  CHECK(probe.snap() == grape::probe::Controller::Error::None);
  CHECK(probe.snap() == grape::probe::Controller::Error::None);
  CHECK(probe.snap() != grape::probe::Controller::Error::None);

  // Check subsequent flush yields expected number of records
  probe.flush();
  CHECK(num_records == BUFFER_CONFIG.snap_buffer_capacity);

  // Check we can qset expected number of times
  static constexpr auto AMPLITUDE_DELTA = 0.1;
  auto amplitude_setting = 0.0;
  CHECK(probe.qset("amplitude", amplitude_setting) == grape::probe::Controller::Error::None);
  amplitude_setting += AMPLITUDE_DELTA;
  CHECK(probe.qset("amplitude", amplitude_setting) == grape::probe::Controller::Error::None);
  CHECK(probe.qset("amplitude", amplitude_setting + AMPLITUDE_DELTA) !=
        grape::probe::Controller::Error::None);

  // Check after sync, the control variable is set to latest value from qset
  probe.sync();
  constexpr auto EPSILON = 0.0001;
  CHECK_THAT(amplitude, Catch::Matchers::WithinRel(amplitude_setting, EPSILON));
}

}  // namespace
