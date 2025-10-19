//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ego_clock_driver.h"

#include "clock_data.h"
#include "grape/ipc/publisher.h"
#include "grape/log/syslog.h"
#include "line_fitter.h"

namespace grape {
struct EgoClockDriver::Impl {
  explicit Impl(const Config& config);
  std::size_t broadcast_interval{ 1U };
  std::size_t count{ 0U };
  ego_clock::LineFitter line_fitter;
  ipc::Publisher<ego_clock::ClockTopic> tick_pub;
};

//-------------------------------------------------------------------------------------------------
EgoClockDriver::Impl::Impl(const Config& config)
  : broadcast_interval(config.broadcast_interval)
  , line_fitter(config.calibration_window)
  , tick_pub(ipc::Publisher(ego_clock::ClockTopic(config.clock_name))) {
  if (broadcast_interval == 0U) {
    panic("Broadcast interval must be greater than 0 ticks");
  }
  if (config.calibration_window < 2U) {
    panic("Calibration window must be at least 2 ticks");
  }
}

//-------------------------------------------------------------------------------------------------
EgoClockDriver::EgoClockDriver(const Config& config) : impl_(std::make_unique<Impl>(config)) {
}

//-------------------------------------------------------------------------------------------------
EgoClockDriver::~EgoClockDriver() = default;

//-------------------------------------------------------------------------------------------------
void EgoClockDriver::tick(const EgoClock::TimePoint& ego_time,
                          const WallClock::TimePoint& wall_time) {
  impl_->line_fitter.add({ .x = static_cast<double>(EgoClock::toNanos(ego_time)),
                           .y = static_cast<double>(WallClock::toNanos(wall_time)) });
  impl_->count++;

  if (impl_->count < impl_->broadcast_interval) {
    return;
  }

  impl_->count = 0U;
  const auto fit = impl_->line_fitter.fit();
  if (not fit) {
    syslog::Error("Failed to fit clock");
    return;
  }

  const auto tf = ego_clock::ClockTransform{ .scale = fit->slope,
                                             .offset = fit->intercept,
                                             .rmse = std::round(std::sqrt(fit->mse)) };
  syslog::Info("Clock fit: {}", toString(tf));
  if (not impl_->tick_pub.publish(tf)) {
    syslog::Error("Failed to publish clock tick");
  }
}

}  // namespace grape
