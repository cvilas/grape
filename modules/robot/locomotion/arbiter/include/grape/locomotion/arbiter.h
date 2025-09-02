//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <functional>
#include <thread>

#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/locomotion/topics.h"
#include "grape/statistics/sliding_mean.h"

namespace grape::locomotion {

//=================================================================================================
/// Arbitrates between locomotion commands from primary and alternate sources.
///
/// - See README for component design considerations
/// - See examples and tests for behaviours and usage
class Arbiter {
public:
  /// Timeout period after which control authority transfers back to primary source when
  /// alternate source stops sending commands
  static constexpr auto ALT_CONTROLLER_TIMEOUT = std::chrono::seconds(2);

  /// Signature of callback to trigger on receiving a command
  using CommandCallback = std::function<void(const Command&)>;

  /// Constructor
  /// @param robot_name Name of robot as used in its IPC topics
  /// @param cmd_cb Callback to trigger on receiving a command
  Arbiter(const std::string& robot_name, CommandCallback&& cmd_cb);

  /// Set primary locomotion command.
  void setPrimary(const Command& cmd) const;

  ~Arbiter() = default;
  Arbiter(const Arbiter&) = delete;
  Arbiter(Arbiter&&) = delete;
  auto operator=(const Arbiter&) -> Arbiter& = delete;
  auto operator=(Arbiter&&) -> Arbiter& = delete;

private:
  static constexpr std::uint64_t NULL_ID = 0UL;
  static constexpr auto LATENCY_TRACKER_WINDOW = 128U;

  void onAlternate(const std::expected<AlternateCommandTopic::DataType, ipc::Error>& cmd,
                   const ipc::SampleInfo& info);
  void publishStatus() const;
  void watchdogLoop(const std::stop_token& stop_token);

  static_assert(std::atomic<SystemClock::TimePoint>::is_always_lock_free);
  static_assert(std::atomic<float>::is_always_lock_free);
  static_assert(std::atomic<std::uint64_t>::is_always_lock_free);

  std::atomic<SystemClock::TimePoint> last_alt_cmd_time_;
  std::atomic<float> cmd_latency_;
  statistics::SlidingMean<float, LATENCY_TRACKER_WINDOW> cmd_latency_tracker_;
  std::atomic<std::uint64_t> alt_controller_id_{ NULL_ID };
  CommandCallback robot_command_cb_{ nullptr };
  ipc::Publisher<ArbiterStatusTopic> status_pub_;
  ipc::Subscriber<AlternateCommandTopic> alt_cmd_sub_;
  std::jthread watchdog_thread_;
};
}  // namespace grape::locomotion
