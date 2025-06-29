//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/robot/loco/topics.h"
#include "grape/statistics/sliding_mean.h"

namespace grape::robot::loco {

//=================================================================================================
/// Locomotion service listens to locomotion commands from autonomous navigation stack and remote
/// teleoperation clients.
///
/// - Locomotion service responds to locomotion commands from two sources
///   - Primary source (via direct send() function call, e.g. from autonomous navigation stack)
///   - Secondary 'teleop' source (via IPC, e.g. from remote teleoperation client)
/// - The service expects a continuous periodic stream of locomotion commands from its active
///   sources. Typically this should be greater than 10 Hz.
/// - Nominally the locomotion service listens to the _primary_ source for locomotion commands.
/// - If a secondary source (e.g. remote teleop client) starts sending a locomotion command
///   stream, the service transfers control authority to that source.
/// - If the secondary source stops sending its command stream, the service transfers control back
///   to the primary source after a timeout period.
/// - At most one secondary source can be active at any time. If multiple secondary sources are
///   sending command streams, the service ignores them from all except one secondary source.
///
class Service {
public:
  /// Timeout period after which control authority transfers back to primary source when
  /// secondary teleop source stops sending commands
  static constexpr auto TELEOP_TIMEOUT = std::chrono::seconds(2);

  /// Signature of callback to trigger on receiving a command
  using CommandCallback = std::function<void(const Command&)>;

  /// Constructor
  /// @param robot_name Name of robot as used in its IPC topic
  /// @param cmd_cb Callback to trigger on receiving a command
  explicit Service(const std::string& robot_name, CommandCallback&& cmd_cb);

  /// Send a locomotion command to the robot. This is the primary source of commands to the robot
  /// @param cmd Desired locomotion command
  void send(const Command& cmd) const;

  ~Service();
  Service(const Service&) = delete;
  Service(Service&&) = delete;
  auto operator=(const Service&) -> Service& = delete;
  auto operator=(Service&&) -> Service& = delete;

private:
  static constexpr std::uint64_t NULL_ID = 0UL;
  static constexpr auto LATENCY_TRACKER_WINDOW = 128U;

  void onTeleopCommand(const ipc::Sample& sample);
  void publishStatus() const;
  void watchdogLoop(const std::stop_token& stop_token);

  static_assert(std::atomic<std::chrono::system_clock::time_point>::is_always_lock_free);
  static_assert(std::atomic<float>::is_always_lock_free);
  static_assert(std::atomic<std::uint64_t>::is_always_lock_free);

  std::atomic<std::chrono::system_clock::time_point> last_teleop_cmd_time_;
  std::atomic<float> teleop_cmd_latency_;
  statistics::SlidingMean<float, LATENCY_TRACKER_WINDOW> teleop_cmd_latency_tracker_;
  std::atomic<std::uint64_t> teleoperator_id_{ NULL_ID };
  CommandCallback robot_command_cb_{ nullptr };
  ipc::Publisher status_pub_;
  ipc::Subscriber teleop_sub_;
  std::jthread watchdog_thread_;
};
}  // namespace grape::robot::loco
