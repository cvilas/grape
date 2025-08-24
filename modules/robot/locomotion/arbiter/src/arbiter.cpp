//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/locomotion/arbiter.h"

#include "grape/log/syslog.h"

namespace grape::locomotion {

//=================================================================================================
Arbiter::Arbiter(const std::string& robot_name, CommandCallback&& robot_cmd_cb)
  : robot_command_cb_(std::move(robot_cmd_cb))
  , status_pub_(ArbiterStatusTopic(robot_name), nullptr)
  , alt_cmd_sub_(AlternateCommandTopic(robot_name),
                 [this](const auto& cmd, const auto& info) { onAlternate(cmd, info); })
  , watchdog_thread_([this](const std::stop_token& token) { watchdogLoop(token); }) {
}

//-------------------------------------------------------------------------------------------------
void Arbiter::onAlternate(const AlternateCommandTopic::DataType& cmd, const ipc::SampleInfo& info) {
  if (alt_controller_id_.load() == NULL_ID) {
    // no alt controller active. take control
    alt_controller_id_.store(info.publisher.id);
    grape::syslog::Note("Alternate '{}' has control", toString(info.publisher));
    publishStatus();
  }

  if (alt_controller_id_.load() != info.publisher.id) {
    // message not from controlling authority. ignore
    return;
  }

  const auto now = SystemClock::now();
  const auto cmd_latency = std::chrono::duration<float>(now - info.publish_time);
  cmd_latency_ = cmd_latency_tracker_.append(cmd_latency.count()).mean;

  // process command
  last_alt_cmd_time_.store(now);
  robot_command_cb_(cmd);
}

//-------------------------------------------------------------------------------------------------
void Arbiter::setPrimary(const Command& cmd) const {
  if (alt_controller_id_.load() != NULL_ID) {
    return;  // Alternate is active. ignore Primary command.
  }

  // pass the command through
  robot_command_cb_(cmd);
}

//-------------------------------------------------------------------------------------------------
void Arbiter::watchdogLoop(const std::stop_token& stop_token) {
  static constexpr auto STATUS_UPDATE_INTERVAL = std::chrono::milliseconds(1000);
  static constexpr auto ALT_CHECK_INTERVAL = STATUS_UPDATE_INTERVAL / 2;
  auto last_status_update_time = SystemClock::now();
  while (not stop_token.stop_requested()) {
    std::this_thread::sleep_for(ALT_CHECK_INTERVAL);

    const auto alt_cntlr_id = alt_controller_id_.load();
    if (alt_cntlr_id == NULL_ID) {
      continue;  // only monitor when Alternate controller is active
    }

    const auto now = SystemClock::now();
    auto publish_status = false;

    if (now - last_alt_cmd_time_.load() > ALT_CONTROLLER_TIMEOUT) {
      alt_controller_id_.store(NULL_ID);
      syslog::Note("Primary has control. Alternate '{:#x}' timed out", alt_cntlr_id);
      cmd_latency_.store(0.F);
      publish_status = true;  // notify status change
    }

    if (now - last_status_update_time > STATUS_UPDATE_INTERVAL) {
      // periodically update when Alternate controller is active
      publish_status = true;
    }

    if (publish_status) {
      last_status_update_time = now;
      publishStatus();
    }
  }
}

//-------------------------------------------------------------------------------------------------
void Arbiter::publishStatus() const {
  const auto avg_latency = std::chrono::duration_cast<SystemClock::Duration>(
      std::chrono::duration<float>(cmd_latency_.load()));
  const auto status = ArbiterStatus{ .alt_controller_id = alt_controller_id_.load(),
                                     .alt_command_latency = avg_latency };
  status_pub_.publish(status);
}

}  // namespace grape::locomotion
