//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <format>
#include <string>

#include "grape/ipc/qos.h"
#include "grape/utils/utils.h"

namespace grape::ipc {

//=================================================================================================
/// Describes a channel for data exchange between IPC endpoints
struct Topic {
  std::string name{ "unset" };
  std::string type_name{ "unset" };
};

//=================================================================================================
/// Defines essential attributes of an IPC topic
template <typename T>
concept TopicAttributes = requires(T obj) {
  // Has an associated data type
  typename T::DataType;

  // Data type must be default constructible
  requires std::default_initializable<typename T::DataType>;

  // Defines quality of service setting
  { T::QOS } -> std::convertible_to<QoS>;

  // Defines max buffer size required to serialise data into raw bytes
  { T::SERDES_BUFFER_SIZE } -> std::convertible_to<std::size_t>;

  // Has a method that returns topic name
  { obj.topicName() } -> std::convertible_to<std::string>;
};

//-------------------------------------------------------------------------------------------------
template <TopicAttributes TopicAttr>
constexpr auto toTopic(const TopicAttr& topic_attr) -> Topic {
  return Topic{
    .name = topic_attr.topicName(),
    .type_name = std::string{ utils::getTypeName<typename TopicAttr::DataType>() },
  };
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Topic& topic) -> std::string {
  return std::format("{} [{}]", topic.name, topic.type_name);
}

}  // namespace grape::ipc
