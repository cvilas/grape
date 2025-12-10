//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "clock_data_receiver.h"

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
auto ClockDataReceiver::transform() -> ClockTransform {
  ClockTransform tf;
  while (true) {
    // Avoid torn reads: retry if writer is active (odd or changed seq)
    const auto seq_before = seq_.load(std::memory_order_relaxed);
    tf = transform_;
    const auto seq_after = seq_.load(std::memory_order_acquire);
    if ((seq_before == seq_after) and ((seq_before & 1U) == 0U)) {
      break;
    }
    std::this_thread::yield();
  }
  return tf;
}

//-------------------------------------------------------------------------------------------------
auto ClockDataReceiver::isInit() const -> bool {
  const auto seq = seq_.load(std::memory_order_acquire);
  return (seq != 0U) && ((seq & 1U) == 0U);
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
  seq_.fetch_add(1, std::memory_order_relaxed);  // Begin write (odd count)
  transform_ = maybe_data.value();
  seq_.fetch_add(1, std::memory_order_release);  // End write (even count)
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
    syslog::Error("Multiple clock drivers detected ({})", num_masters_.load());
  } else if (num_masters_ == 0U) {
    syslog::Error("No clock driver available");
  }
}

}  // namespace grape::ego_clock
