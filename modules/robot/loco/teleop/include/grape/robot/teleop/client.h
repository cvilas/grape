//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <string>

#include "grape/robot/loco/types.h"
#include "grape/robot/teleop/status.h"

namespace grape::robot::teleop {

//=================================================================================================
class Client {
public:
  using StatusCallback = std::function<void(const Status&)>;
  explicit Client(const std::string& robot_name, StatusCallback&& status_cb);
  void acquire();
  void release();
  void command(const loco::Command& cmd);

private:
  StatusCallback status_callback_{ nullptr };
};
}  // namespace grape::robot::teleop
