//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/topic_attributes.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace grape::ipc {

//=================================================================================================
/// Publisher templated on topic attributes
///
template <typename TopicAttributes>
class Publisher : public RawPublisher {
public:
  explicit Publisher(const TopicAttributes& topic_attr, MatchCallback&& match_cb = nullptr);
  [[nodiscard]] auto publish(const typename TopicAttributes::DataType& data) const
      -> std::expected<void, Error>;
};

//-------------------------------------------------------------------------------------------------
template <typename TopicAttributes>
Publisher<TopicAttributes>::Publisher(const TopicAttributes& topic_attr, MatchCallback&& match_cb)
  : RawPublisher(topic_attr.topicName(), std::move(match_cb)) {
}

//-------------------------------------------------------------------------------------------------
template <typename TopicAttributes>
auto Publisher<TopicAttributes>::publish(const typename TopicAttributes::DataType& data) const
    -> std::expected<void, Error> {
  auto stream = serdes::OutStream<TopicAttributes::SERDES_BUFFER_SIZE>{};
  auto ser = serdes::Serialiser(stream);
  if (not ser.pack(data)) {
    return std::unexpected{ Error::SerialisationFailed };
  }
  return RawPublisher::publish(stream.data());
}
}  // namespace grape::ipc
