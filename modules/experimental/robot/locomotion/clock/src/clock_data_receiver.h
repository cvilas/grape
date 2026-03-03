//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>

#include "clock_topic.h"
#include "grape/ipc/subscriber.h"

namespace grape::ego_clock {

//=================================================================================================
/// Receives clock ticks from ego clock driver
class ClockDataReceiver {
public:
  explicit ClockDataReceiver(const std::string& clock_name);
  [[nodiscard]] auto isInit() const -> bool;
  [[nodiscard]] auto transform() -> ClockTransform;

private:
  void onMatch(const ipc::Match& match);
  void onTick(const std::expected<ClockTransform, ipc::Error>& maybe_data,
              const ipc::SampleInfo& info);

  std::atomic<std::size_t> num_masters_{ 0U };
  std::atomic<std::uint64_t> seq_{ 0U };
  ClockTransform transform_;
  ipc::Subscriber<ClockTopic> tick_sub_;
};

}  // namespace grape::ego_clock
