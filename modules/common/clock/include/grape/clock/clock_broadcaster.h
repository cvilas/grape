//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>

#include "grape/clock/follower_clock.h"

namespace grape::clock {

//=================================================================================================
/// Broadcasts clock ticks to FollowerClock instances on the host
///
class ClockBroadcaster {
public:
  struct Config {
    std::string name;  //!< Uniquely identifies the broadcaster
  };

  /// Construct and start the driver
  /// @param config Configuration parameters
  explicit ClockBroadcaster(Config config);

  /// Assert a tick from the master clock
  /// @param tp Time reference in user-defined time source
  void post(const FollowerClock::TimePoint& tp);

  ~ClockBroadcaster();
  ClockBroadcaster(const ClockBroadcaster&) = delete;
  ClockBroadcaster(ClockBroadcaster&&) = delete;
  auto operator=(const ClockBroadcaster&) = delete;
  auto operator=(ClockBroadcaster&&) = delete;

private:
  Config config_;
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape::clock
