//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <string>

namespace grape::ipc::ex {
struct ExampleTopicAttributes {
  using DataType = std::string;
  static constexpr auto QOS = QoS::BestEffort;
  static constexpr auto SERDES_BUFFER_SIZE = 256U;
  static constexpr auto TOPIC = "/topic_example";
  static auto topicName() -> std::string {
    return TOPIC;
  }
};
}  // namespace grape::ipc::ex
