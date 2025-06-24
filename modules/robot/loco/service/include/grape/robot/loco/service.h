//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/robot/loco/topics.h"

namespace grape::robot::loco {

//=================================================================================================
class Service {
public:
  using CommandCallback = std::function<void(const Command&)>;
  explicit Service(const std::string& robot_name, CommandCallback&& cmd_cb);

  ~Service();
  Service(const Service&) = delete;
  Service(Service&&) = delete;
  auto operator=(const Service&) -> Service& = delete;
  auto operator=(Service&&) -> Service& = delete;

private:
  static constexpr auto TELEOP_TIMEOUT = std::chrono::seconds(2);
  static constexpr auto WATCHDOG_INTERVAL = std::chrono::milliseconds(500);
  static constexpr std::uint64_t INVALID_ID = 0UL;

  void onTeleopCommand(const ipc::Sample& sample);
  void onAutonavCommand(const ipc::Sample& sample);
  void publishStatus() const;
  void watchdogLoop(const std::stop_token& stop_token);

  std::atomic<std::chrono::system_clock::time_point> last_teleop_cmd_time_;
  std::atomic<std::uint64_t> teleoperator_id_{ INVALID_ID };
  CommandCallback robot_command_cb_{ nullptr };
  std::unique_ptr<ipc::Subscriber> teleop_sub_{ nullptr };
  std::unique_ptr<ipc::Subscriber> autonav_sub_{ nullptr };
  std::unique_ptr<ipc::Publisher> status_pub_{ nullptr };
  std::jthread watchdog_thread_;
};
}  // namespace grape::robot::loco
