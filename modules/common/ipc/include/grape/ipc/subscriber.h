//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <expected>

#include "grape/ipc/error.h"
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
  using DataCallback = std::function<void(
      const std::expected<typename TopicAttributes::DataType, Error>&, const SampleInfo&)>;

  Subscriber(const TopicAttributes& topic_attr, DataCallback&& data_cb,
             MatchCallback&& match_cb = nullptr);
};

//-------------------------------------------------------------------------------------------------
template <typename TopicAttributes>
Subscriber<TopicAttributes>::Subscriber(const TopicAttributes& topic_attr, DataCallback&& data_cb,
                                        MatchCallback&& match_cb)
  : RawSubscriber(
        topic_attr.topicName(),
        [moved_data_cb = std::move(data_cb)](const Sample& sample) {
          if (moved_data_cb == nullptr) {
            return;
          }
          auto result = std::expected<typename TopicAttributes::DataType, Error>{ std::in_place };
          auto stream = serdes::InStream(sample.data);
          auto deserialiser = serdes::Deserialiser(stream);
          if (not deserialiser.unpack(*result)) {
            result = std::unexpected{ Error::DeserialisationFailed };
          }
          moved_data_cb(result, sample.info);
        },
        std::move(match_cb)) {
}
}  // namespace grape::ipc
