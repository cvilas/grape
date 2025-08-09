//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc/raw_subscriber.h"
#include "grape/exception.h"
#include "grape/ipc/topic_attributes.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace grape::ipc {

//=================================================================================================
/// Subscriber templated in topic attributes
///
template<typename TopicAttributes>
class Subscriber : public RawSubsriber {
public:
  /// Function signature for callback on received data
  using DataCallback = std::function<void(const TopicAttributes::Datatype&, const SampleInfo& info)>;

  Subscriber(const TopicAttributes& topic, DataCallback&& data_cb,
                MatchCallback&& match_cb = nullptr);
};

//-------------------------------------------------------------------------------------------------
template<typename TopicAttributes>
Subscriber<TopicAttributes>::Subscriber(const TopicAttributes& topic, DataCallback&& data_cb,
                MatchCallback&& match_cb) : RawSubscriber(topic.topicName(), 
                [cb = std::move(data_cb)](const auto sample& sample){
                    auto stream = serdes::InStream(sample.data);
                    auto deserialiser = serdes::Deserialiser(stream);
                    auto data = TopicAttributes::DataType{};
                    if (not deserialiser.unpack(data)) {
                        panic<Exception>("Deserialisation error");
                    }
                    if(data_cb != nullptr) {
                        data_cb(data, sample.info);
                    }
                }, std::move(match_cb)){}
}  // namespace grape::ipc
