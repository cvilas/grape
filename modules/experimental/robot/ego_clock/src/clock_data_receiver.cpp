//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "clock_data_receiver.h"

#include <cmath>
#include <thread>

#include "grape/log/syslog.h"

namespace grape::ego_clock {

//-------------------------------------------------------------------------------------------------
ClockDataReceiver::ClockDataReceiver(const std::string& clock_name)
  : tick_sub_(
        ClockTopic(clock_name), [this](const auto& data, const auto& info) { onTick(data, info); },
        [this](const auto& match) { onMatch(match); }) {
}

//-------------------------------------------------------------------------------------------------
auto ClockDataReceiver::transform() const -> std::optional<ClockTransform> {
  const auto idx = readable_index_.load(std::memory_order_acquire);
  if (idx < 0) {
    return std::nullopt;
  }
  return buffers_.at(static_cast<std::size_t>(idx));
}

//-------------------------------------------------------------------------------------------------
void ClockDataReceiver::onTick(const std::expected<ClockTransform, ipc::Error>& maybe_data,
                               const ipc::SampleInfo& info) {
  (void)info;
  if (num_masters_ != 1U) {
    return;
  }
  if (not maybe_data) {
    syslog::Error("Error receiving clock data: {}", toString(maybe_data.error()));
    return;
  }
  buffers_.at(writable_index_) = maybe_data.value();
  readable_index_.store(static_cast<int>(writable_index_), std::memory_order_release);
  writable_index_ = (1 - writable_index_);  // swap writable buffer
}

//-------------------------------------------------------------------------------------------------
void ClockDataReceiver::onMatch(const ipc::Match& match) {
  switch (match.status) {
    case ipc::Match::Status::Matched:
      ++num_masters_;
      break;
    case ipc::Match::Status::Unmatched:
      if (num_masters_ > 0U) {
        --num_masters_;
      }
      break;
  }
  syslog::Info("Clock driver {}: {} (total {})", toString(match.status),
               toString(match.remote_entity), num_masters_.load());

  if (num_masters_ > 1U) {
    readable_index_.store(-1, std::memory_order_release);  // Invalidate data
    syslog::Error("Multiple clock drivers detected ({})", num_masters_.load());
  } else if (num_masters_ == 0U) {
    readable_index_.store(-1, std::memory_order_release);  // Invalidate data
    syslog::Error("No clock driver available");
  }
}

}  // namespace grape::ego_clock
