//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ego_clock_driver.h"

#include "clock_topic.h"
#include "grape/ipc/publisher.h"
#include "grape/log/syslog.h"
#include "line_fitter.h"

namespace grape {
struct EgoClockDriver::Impl {
  explicit Impl(const Config& config);
  WallClock::Duration broadcast_interval{};
  WallClock::TimePoint last_broadcast_time;
  ego_clock::ClockTransform last_fit;
  ego_clock::LineFitter line_fitter;
  ipc::Publisher<ego_clock::ClockTopic> tick_pub;
};

//-------------------------------------------------------------------------------------------------
EgoClockDriver::Impl::Impl(const Config& config)
  : broadcast_interval(config.broadcast_interval)
  , line_fitter(config.calibration_window)
  , tick_pub(ipc::Publisher(ego_clock::ClockTopic(config.clock_name))) {
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
  if (wall_time < impl_->last_broadcast_time + impl_->broadcast_interval) {
    return;
  }
  impl_->last_broadcast_time = wall_time;

  const auto fit = impl_->line_fitter.fit();
  if (not fit) {
    syslog::Note("No WallClock/EgoClock correlation available yet");
    return;
  }

  const auto tf = ego_clock::ClockTransform{ .scale = fit->slope,
                                             .offset = fit->intercept,
                                             .rmse = std::round(std::sqrt(fit->mse)) };
  if (not impl_->tick_pub.publish(tf)) {
    syslog::Error("Failed to publish clock tick");
  }

  // Check against last fit
  if (impl_->last_fit.rmse > 0.) {
    const auto prev_fit = ego_clock::toWallTime(impl_->last_fit, ego_time);
    const auto now_fit = ego_clock::toWallTime(tf, ego_time);
    const auto delta_fit = now_fit - prev_fit;
    static constexpr auto THRESHOLD = WallClock::Duration(std::chrono::milliseconds(1));
    if (delta_fit > THRESHOLD) {
      syslog::Info("Clock fit: {}", toString(tf));
      syslog::Warn("Deviation from last fit (={}) crosses threshold (={})", delta_fit, THRESHOLD);
    }
  }
  impl_->last_fit = tf;
}

}  // namespace grape
