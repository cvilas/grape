//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/exception.h"
#include "grape/ipc/raw_subscriber.h"
#include "grape/ipc/topic_attributes.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace grape::ipc {

//=================================================================================================
/// Subscriber templated in topic attributes
///
template <typename TopicAttributes>
class Subscriber : public RawSubscriber {
public:
  using DataCallback =
      std::function<void(const typename TopicAttributes::DataType&, const SampleInfo&)>;

  Subscriber(const TopicAttributes& topic_attr, DataCallback&& data_cb,
             MatchCallback&& match_cb = nullptr);
};

//-------------------------------------------------------------------------------------------------
template <typename TopicAttributes>
Subscriber<TopicAttributes>::Subscriber(const TopicAttributes& topic_attr, DataCallback&& data_cb,
                                        MatchCallback&& match_cb)
  : RawSubscriber(
        topic_attr.topicName(),
        [moved_data_cb = std::move(data_cb)](const Sample& sample) -> void {
          auto stream = serdes::InStream(sample.data);
          auto deserialiser = serdes::Deserialiser(stream);
          typename TopicAttributes::DataType data{};
          if (not deserialiser.unpack(data)) {
            panic<Exception>("Deserialisation error");
          }
          if (moved_data_cb != nullptr) {
            moved_data_cb(data, sample.info);
          }
        },
        std::move(match_cb)) {
}
}  // namespace grape::ipc
