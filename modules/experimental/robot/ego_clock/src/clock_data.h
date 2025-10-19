//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <format>

#include "grape/ipc/qos.h"
#include "grape/serdes/serdes.h"

namespace grape::ego_clock {

//=================================================================================================
/// Data for fitting correspondance between ego-clock and wall-clock
/// wall_clock_ns = scale * ego_clock_ns + offset
struct ClockTransform {
  double scale{ 0. };
  double offset{ 0. };
  double rmse{ 0. };
};

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

//-------------------------------------------------------------------------------------------------
constexpr auto toString(const ClockTransform& tf) -> std::string {
  return std::format("scale={}, offset={}, rmse={}", tf.scale, tf.offset, tf.rmse);
}

//=================================================================================================
struct ClockTopic {
public:
  using DataType = ClockTransform;
  static constexpr auto TOPIC_SUFFIX = "/clock";
  static constexpr auto QOS = ipc::QoS::BestEffort;
  static constexpr auto SERDES_BUFFER_SIZE = 2 * sizeof(ClockTransform);
  [[nodiscard]] static auto topicName() -> std::string {
    return TOPIC_SUFFIX;
  }
};

}  // namespace grape::ego_clock
