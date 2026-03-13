//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/topic.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace grape::ipc {

//=================================================================================================
/// Publisher templated on topic attributes
///
template <TopicAttributes TopicAttr>
class Publisher : public RawPublisher {
public:
  explicit Publisher(const TopicAttr& topic_attr, MatchCallback&& match_cb = nullptr);
  [[nodiscard]] auto publish(const TopicAttr::DataType& data) const -> std::expected<void, Error>;
};

//-------------------------------------------------------------------------------------------------
template <TopicAttributes TopicAttr>
Publisher<TopicAttr>::Publisher(const TopicAttr& topic_attr, MatchCallback&& match_cb)
  : RawPublisher(toTopic(topic_attr), std::move(match_cb)) {
}

//-------------------------------------------------------------------------------------------------
template <TopicAttributes TopicAttr>
auto Publisher<TopicAttr>::publish(const TopicAttr::DataType& data) const
    -> std::expected<void, Error> {
  auto stream = serdes::OutStream<TopicAttr::SERDES_BUFFER_SIZE>{};
  auto ser = serdes::Serialiser(stream);
  if (not ser.pack(data)) {
    return std::unexpected{ Error::SerialisationFailed };
  }
  return RawPublisher::publish(stream.data());
}
}  // namespace grape::ipc
