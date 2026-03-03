//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>

#include "grape/ego_clock2.h"

namespace grape {

//=================================================================================================
/// Drives EgoClock2 consumers
///
/// Posts ego clock ticks directly to shared memory using futex-based signaling. Unlike
/// EgoClockDriver, no line fitting or IPC broadcasting is performed; each tick is posted
/// immediately with nanosecond precision to all waiting consumers.
class EgoClock2Driver {
public:
  struct Config {
    std::string clock_name;  //!< Shared memory name for the clock (must start with '/')
  };

  /// Construct and start the driver
  /// @param config Configuration parameters
  explicit EgoClock2Driver(const Config& config);

  /// Post a clock tick to all waiting consumers
  /// @param ego_time Current ego clock time
  void tick(const EgoClock2::TimePoint& ego_time);

  ~EgoClock2Driver();
  EgoClock2Driver(const EgoClock2Driver&) = delete;
  EgoClock2Driver(EgoClock2Driver&&) = delete;
  auto operator=(const EgoClock2Driver&) = delete;
  auto operator=(EgoClock2Driver&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape
