//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <atomic>

#include "clock_data.h"
#include "grape/ipc/subscriber.h"

namespace grape::ego_clock {

//=================================================================================================
/// Receives clock ticks from ego clock driver
class ClockDataReceiver {
public:
  explicit ClockDataReceiver(const std::string& clock_name);
  [[nodiscard]] auto transform() const -> std::optional<ClockTransform>;

private:
  void onMatch(const ipc::Match& match);
  void onTick(const std::expected<ClockTransform, ipc::Error>& maybe_data,
              const ipc::SampleInfo& info);

  std::atomic<std::size_t> num_masters_{ 0U };
  std::array<ClockTransform, 2> buffers_;
  std::atomic<int> readable_index_{ -1 };  // -1 = no data, 0/1 = valid buffer index
  std::size_t writable_index_{ 0 };        // Only writer modifies this
  ipc::Subscriber<ClockTopic> tick_sub_;
};

}  // namespace grape::ego_clock
