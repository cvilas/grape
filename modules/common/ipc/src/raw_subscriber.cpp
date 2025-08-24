//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/raw_subscriber.h"

#include <ecal/pubsub/subscriber.h>
#include <ecal/pubsub/types.h>

#include "grape/exception.h"
#include "grape/ipc/session.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto toMatchEvent(const eCAL::STopicId& topic_id, const eCAL::SSubEventCallbackData& event_data)
    -> grape::ipc::Match {
  auto match_status = grape::ipc::Match::Status::Undefined;
  switch (event_data.event_type) {
    case eCAL::eSubscriberEvent::none:
      match_status = grape::ipc::Match::Status::Undefined;
      break;
    case eCAL::eSubscriberEvent::connected:
      match_status = grape::ipc::Match::Status::Matched;
      break;
    case eCAL::eSubscriberEvent::disconnected:
      [[fallthrough]];
    case eCAL::eSubscriberEvent::dropped:
      match_status = grape::ipc::Match::Status::Unmatched;
      break;
  }
  return { .remote_entity = { .host = topic_id.topic_id.host_name,
                              .id = topic_id.topic_id.entity_id },
           .status = match_status };
}
}  // namespace

namespace grape::ipc {

struct RawSubscriber::Impl : public eCAL::CSubscriber {
  Impl(const std::string& topic_name, const eCAL::SubEventCallbackT& event_cb)
    : eCAL::CSubscriber(topic_name, eCAL::SDataTypeInformation(), event_cb) {
  }
};

//-------------------------------------------------------------------------------------------------
RawSubscriber::RawSubscriber(const std::string& topic, RawSubscriber::DataCallback&& data_cb,
                             MatchCallback&& match_cb) {
  if (not ok()) {
    panic<Exception>("Not initialised");
  }

  const auto event_cb = [moved_match_cb = std::move(match_cb)](
                            const eCAL::STopicId& topic_id,
                            const eCAL::SSubEventCallbackData& event_data) -> void {
    if (moved_match_cb != nullptr) {
      moved_match_cb(toMatchEvent(topic_id, event_data));
    }
  };

  impl_ = std::make_unique<RawSubscriber::Impl>(topic, event_cb);

  impl_->SetReceiveCallback([moved_data_cb = std::move(data_cb)](
                                const eCAL::STopicId& id, const eCAL::SDataTypeInformation&,
                                const eCAL::SReceiveCallbackData& data) -> void {
    const auto tp = SystemClock::TimePoint(SystemClock::Duration{ data.send_timestamp });
    if (moved_data_cb != nullptr) {
      moved_data_cb({ .data = { static_cast<const std::byte*>(data.buffer), data.buffer_size },
                      .info = { .publish_time = tp,
                                .publisher = { .host = id.topic_id.host_name,
                                               .id = id.topic_id.entity_id } } });
    }
  });
}

//-------------------------------------------------------------------------------------------------
RawSubscriber::~RawSubscriber() = default;

//-------------------------------------------------------------------------------------------------
RawSubscriber::RawSubscriber(RawSubscriber&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
auto RawSubscriber::publisherCount() const -> std::size_t {
  return impl_->GetPublisherCount();
}

//-------------------------------------------------------------------------------------------------
auto RawSubscriber::id() const -> std::uint64_t {
  return impl_->GetTopicId().topic_id.entity_id;
}

}  // namespace grape::ipc
