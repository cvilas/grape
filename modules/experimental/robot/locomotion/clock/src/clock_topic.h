//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "clock_transform.h"
#include "grape/ipc/qos.h"
#include "grape/serdes/serdes.h"

namespace grape::ego_clock {

//-------------------------------------------------------------------------------------------------
template <serdes::WritableStream S>
constexpr auto serialise(serdes::Serialiser<S>& ser, const ClockTransform& data) -> bool {
  return ser.pack(data.scale) and ser.pack(data.offset) and ser.pack(data.rmse);
}

//-------------------------------------------------------------------------------------------------
template <serdes::ReadableStream S>
constexpr auto deserialise(serdes::Deserialiser<S>& des, ClockTransform& data) -> bool {
  return des.unpack(data.scale) and des.unpack(data.offset) and des.unpack(data.rmse);
}

//=================================================================================================
class ClockTopic {
public:
  using DataType = ClockTransform;
  static constexpr auto TOPIC_SUFFIX = "/clock";
  static constexpr auto QOS = ipc::QoS::BestEffort;
  static constexpr auto SERDES_BUFFER_SIZE = 2 * sizeof(ClockTransform);
  explicit ClockTopic(const std::string& clock_name) : topic_name_(clock_name + TOPIC_SUFFIX) {
  }
  [[nodiscard]] auto topicName() const -> const std::string& {
    return topic_name_;
  }

private:
  std::string topic_name_;
};

}  // namespace grape::ego_clock
