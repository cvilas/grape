//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/exception.h"
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
  void publish(const typename TopicAttributes::DataType& data) const;
};

//-------------------------------------------------------------------------------------------------
template <typename TopicAttributes>
Publisher<TopicAttributes>::Publisher(const TopicAttributes& topic_attr, MatchCallback&& match_cb)
  : RawPublisher(topic_attr.topicName(), std::move(match_cb)) {
}

//-------------------------------------------------------------------------------------------------
template <typename TopicAttributes>
void Publisher<TopicAttributes>::publish(const typename TopicAttributes::DataType& data) const {
  auto stream = serdes::OutStream<TopicAttributes::SERDES_BUFFER_SIZE>{};
  auto ser = serdes::Serialiser(stream);
  if (not ser.pack(data)) {
    panic<Exception>("Serialisation error");
  }
  RawPublisher::publish(stream.data());
}
}  // namespace grape::ipc
