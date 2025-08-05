//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <string>

#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/locomotion/topics.h"

namespace grape::locomotion {

//=================================================================================================
/// Teleoperation client interface. Sends locomotion commands over the alternate command channel to
/// to the robot locomotion stack.
class TeleopClient {
public:
  /// Status of the teleoperation client
  struct Status {
    bool is_service_detected{ false };  //!< true if locomotion service endpoint is detected
    bool is_client_active{ false };     //!< true if this client has control authority
    std::chrono::system_clock::duration command_latency{};  //!< Avg. latency to locomotion service
  };

  /// Callback function signature for receiving teleoperation status updates
  using StatusCallback = std::function<void(const Status&)>;

  /// Creates a teleoperation client
  /// @param robot_name Name of the robot to connect to
  /// @param status_cb Callback function to receive teleoperation status updates
  TeleopClient(const std::string& robot_name, StatusCallback&& status_cb);

  /// Send a locomotion command to the robot
  /// @param cmd Locomotion command to send
  void send(const AlternateCommandTopic::DataType& cmd);

private:
  void onArbiterMatch(const ipc::Match& match) const;
  void onArbiterStatus(const ArbiterStatus& status, const ipc::SampleInfo& info) const;
  std::uint64_t id_{ 0 };
  StatusCallback status_cb_{ nullptr };
  grape::ipc::Subscriber<ArbiterStatusTopic> arbiter_status_sub_;
  grape::ipc::Publisher<AlternateCommandTopic> cmd_pub_;
};
}  // namespace grape::locomotion
