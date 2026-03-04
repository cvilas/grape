//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>

#include "grape/follower_clock.h"

namespace grape {

//=================================================================================================
/// Drives follower clocks
///
/// The driver must be driven by a user-defined source, e.g. a real-time operating system,
/// hardware clock, or physics simulation.
class ClockDriver {
public:
  struct Config {
    std::string clock_name;  //!< Uniquely identifies clock source
  };

  /// Construct and start the driver
  /// @param config Configuration parameters
  explicit ClockDriver(const Config& config);

  /// Assert a tick from the master clock
  /// @param tp Time reference in user-defined time source
  void tick(const FollowerClock::TimePoint& tp);

  ~ClockDriver();
  ClockDriver(const ClockDriver&) = delete;
  ClockDriver(ClockDriver&&) = delete;
  auto operator=(const ClockDriver&) = delete;
  auto operator=(ClockDriver&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape
