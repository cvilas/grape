//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <string>

namespace grape::ipc {

//=================================================================================================
/// Defines essential attributes of an IPC topic
template <typename T>
concept TopicAttributes = requires(T obj) {
  typename T::DataType;  //!< data type carried on a topic
  { T::SERDES_BUFFER_SIZE } -> std::convertible_to<std::size_t>; //!< Data serialisation buffer size
  { obj.topicName() } -> std::convertible_to<std::string>;
};

}  // namespace grape::ipc
