//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <string>

namespace grape::ipc {

//=================================================================================================
/// Defines essential attributes of an IPC topic
template <typename T>
concept TopicAttributes = requires(T obj) {
  // Has an associated data type
  typename T::DataType;

  // Data type must be default constructible
  requires std::default_initializable<typename T::DataType>;

  // Defines max buffer size required to serialise data into raw bytes
  { T::SERDES_BUFFER_SIZE } -> std::convertible_to<std::size_t>;

  // Has a method that returns topic name
  { obj.topicName() } -> std::convertible_to<std::string>;
};

}  // namespace grape::ipc
