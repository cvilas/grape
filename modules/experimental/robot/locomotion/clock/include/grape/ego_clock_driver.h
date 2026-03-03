//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>

#include "grape/ego_clock.h"
#include "grape/wall_clock.h"

namespace grape {

//=================================================================================================
/// Drives ego clocks
///
/// The driver must be driven by a user-defined source, e.g. a real-time operating system,
/// hardware clock, or physics simulation.
class EgoClockDriver {
public:
  struct Config {
    std::string clock_name;                    //!< Uniquely identifies clock source
    WallClock::Duration broadcast_interval{};  //!< Interval between system-wide clock sync tx
    std::size_t calibration_window{ 2U };  //!< Number of tick samples used to evaluate clock fit
  };

  /// Construct and start the driver
  /// @param config Configuration parameters
  explicit EgoClockDriver(const Config& config);

  /// Assert a tick from the master clock
  /// @param ego_time Time reference in user-defined time source
  /// @param wall_time Corresponding wall clock time (UTC)
  void tick(const EgoClock::TimePoint& ego_time, const WallClock::TimePoint& wall_time);

  ~EgoClockDriver();
  EgoClockDriver(const EgoClockDriver&) = delete;
  EgoClockDriver(EgoClockDriver&&) = delete;
  auto operator=(const EgoClockDriver&) = delete;
  auto operator=(EgoClockDriver&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};
}  // namespace grape
