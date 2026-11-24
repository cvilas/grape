//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc/raw_publisher.h"
#include "grape/ipc/topic.h"
#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"
#include "grape/utils/scope_guard.h"

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

template <TopicAttributes TopicAttr>
Publisher(const TopicAttr&, MatchCallback&& = nullptr) -> Publisher<TopicAttr>;

//-------------------------------------------------------------------------------------------------
template <TopicAttributes TopicAttr>
Publisher<TopicAttr>::Publisher(const TopicAttr& topic_attr, MatchCallback&& match_cb)
  : RawPublisher(toTopic(topic_attr), std::move(match_cb)) {
}

//-------------------------------------------------------------------------------------------------
template <TopicAttributes TopicAttr>
auto Publisher<TopicAttr>::publish(const TopicAttr::DataType& data) const
    -> std::expected<void, Error> {
  using Ostream = serdes::OutStream<TopicAttr::SERDES_BUFFER_SIZE>;
  struct ThreadStream {
    Ostream stream;
    bool active{ false };
  };
  thread_local auto thread_stream = ThreadStream{};

  const auto serialise_and_publish = [this, &data](Ostream& stream) -> std::expected<void, Error> {
    stream.reset();
    auto serialiser = serdes::Serialiser(stream);
    if (not serialiser.pack(data)) {
      return std::unexpected{ Error::SerialisationFailed };
    }
    return RawPublisher::publish(stream.data());
  };

  if (thread_stream.active) {
    // Same-thread reentrant call. create temporary buffer and publish
    auto stream = Ostream{};
    return serialise_and_publish(stream);
  }

  thread_stream.active = true;
  auto* active = &thread_stream.active;
  const auto guard = ScopeGuard{ [active] { *active = false; } };
  return serialise_and_publish(thread_stream.stream);
}
}  // namespace grape::ipc
