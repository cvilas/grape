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
  struct ServiceStatus {
    bool is_detected{ false };  //!< true if locomotion service endpoint is detected
  };

  struct Error {
    std::string message;  //!< Description of any internal communication errors
  };

  struct ClientStatus {
    bool is_client_active{ false };           //!< true if this client has control authority
    SystemClock::Duration command_latency{};  //!< Avg. latency to locomotion service
  };

  /// Callback function signature for receiving status updates
  using Status = std::variant<ServiceStatus, ClientStatus, Error>;
  using StatusCallback = std::function<void(const Status&)>;

  /// Creates a teleoperation client
  /// @param robot_name Name of the robot to connect to
  /// @param status_cb Callback function to receive teleoperation status updates
  TeleopClient(const std::string& robot_name, StatusCallback&& status_cb);

  /// Send a locomotion command to the robot
  /// @param cmd Locomotion command to send
  [[nodiscard]] auto send(const AlternateCommandTopic::DataType& cmd) -> bool;

private:
  void onArbiterMatch(const ipc::Match& match) const;
  void onArbiterStatus(const std::expected<ArbiterStatus, ipc::Error>& maybe_status,
                       const ipc::SampleInfo& info) const;
  std::uint64_t id_{ 0 };
  StatusCallback status_cb_{ nullptr };
  grape::ipc::Subscriber<ArbiterStatusTopic> arbiter_status_sub_;
  grape::ipc::Publisher<AlternateCommandTopic> cmd_pub_;
};
}  // namespace grape::locomotion
