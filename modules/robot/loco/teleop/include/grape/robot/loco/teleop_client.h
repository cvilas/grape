//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <string>

#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/robot/loco/topics.h"

namespace grape::robot::loco {

//=================================================================================================
/// Client interface for remote teleoperation. Communicates with loco::Service
///
/// See loco::Service for details on how the teleoperation service works. This client interface
/// can send locomotion commands to the robot and receive status updates from locomotion service.
///
class TeleopClient {
public:
  /// Status of the teleoperation client
  struct Status {
    bool is_service_detected{ false };  //!< The loco service endpoint is detected and matched
    bool is_client_active{ false };     //!< This teleoperation client has control authority
    std::chrono::system_clock::duration command_latency{};  //!< Avg. latency from client to server
  };

  /// Callback function signature for receiving teleoperation status updates
  using StatusCallback = std::function<void(const Status&)>;

  /// Creates a teleoperation client
  /// @param robot_name Name of the robot to connect to
  /// @param status_cb Callback function to receive teleoperation status updates
  TeleopClient(const std::string& robot_name, StatusCallback&& status_cb);

  /// Send a locomotion command to the robot
  /// @param cmd Locomotion command to send
  void send(const loco::AlternateCommandTopic::DataType& cmd);

private:
  void onArbiterMatch(const ipc::Match& match) const;
  void onArbiterStatus(const ipc::Sample& sample) const;
  std::uint64_t id_{ 0 };
  StatusCallback status_cb_{ nullptr };
  grape::ipc::Subscriber arbiter_status_sub_;
  grape::ipc::Publisher cmd_pub_;
};
}  // namespace grape::robot::loco
