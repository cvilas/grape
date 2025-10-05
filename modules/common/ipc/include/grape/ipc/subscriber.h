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
template <TopicAttributes Topic>
class Subscriber : public RawSubscriber {
public:
  using DataCallback =
      std::function<void(const std::expected<typename Topic::DataType, Error>&, const SampleInfo&)>;

  Subscriber(const Topic& topic_attr, DataCallback&& data_cb, MatchCallback&& match_cb = nullptr);
};

//-------------------------------------------------------------------------------------------------
template <TopicAttributes Topic>
Subscriber<Topic>::Subscriber(const Topic& topic_attr, DataCallback&& data_cb,
                              MatchCallback&& match_cb)
  : RawSubscriber(
        topic_attr.topicName(), Topic::QOS,
        [moved_data_cb = std::move(data_cb)](const Sample& sample) {
          if (not moved_data_cb) {
            return;
          }
          auto result = std::expected<typename Topic::DataType, Error>{ std::in_place };
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
