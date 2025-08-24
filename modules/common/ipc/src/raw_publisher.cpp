//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/raw_publisher.h"

#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/types.h>

#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/time.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto toMatchEvent(const eCAL::STopicId& topic_id, const eCAL::SPubEventCallbackData& event_data)
    -> grape::ipc::Match {
  auto match_status = grape::ipc::Match::Status::Undefined;
  switch (event_data.event_type) {
    case eCAL::ePublisherEvent::none:
      match_status = grape::ipc::Match::Status::Undefined;
      break;
    case eCAL::ePublisherEvent::connected:
      match_status = grape::ipc::Match::Status::Matched;
      break;
    case eCAL::ePublisherEvent::disconnected:
      [[fallthrough]];
    case eCAL::ePublisherEvent::dropped:
      match_status = grape::ipc::Match::Status::Unmatched;
      break;
  }
  return { .remote_entity = { .host = topic_id.topic_id.host_name,
                              .id = topic_id.topic_id.entity_id },
           .status = match_status };
}
}  // namespace

namespace grape::ipc {

struct RawPublisher::Impl : public eCAL::CPublisher {
  Impl(const std::string& topic_name, const eCAL::PubEventCallbackT& event_cb)
    : eCAL::CPublisher(topic_name, eCAL::SDataTypeInformation(), event_cb) {
  }
};

//-------------------------------------------------------------------------------------------------
RawPublisher::RawPublisher(const std::string& topic, MatchCallback&& match_cb) {
  if (not ok()) {
    panic<Exception>("Not initialised");
  }
  const auto event_cb = [moved_match_cb = std::move(match_cb)](
                            const eCAL::STopicId& topic_id,
                            const eCAL::SPubEventCallbackData& event_data) -> void {
    if (moved_match_cb != nullptr) {
      moved_match_cb(toMatchEvent(topic_id, event_data));
    }
  };
  impl_ = std::make_unique<RawPublisher::Impl>(topic, event_cb);
}

//-------------------------------------------------------------------------------------------------
RawPublisher::~RawPublisher() = default;

//-------------------------------------------------------------------------------------------------
RawPublisher::RawPublisher(RawPublisher&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
void RawPublisher::publish(std::span<const std::byte> bytes) const {
  const auto now = SystemClock::now().time_since_epoch().count();
  if (not impl_->Send(bytes.data(), bytes.size(), now)) {
    if (impl_->GetSubscriberCount() > 0U) {
      panic<Exception>(std::format("Write failed on topic '{}'", impl_->GetTopicId().topic_name));
    }
  }
}

//-------------------------------------------------------------------------------------------------
auto RawPublisher::subscriberCount() const -> std::size_t {
  return impl_->GetSubscriberCount();
}

//-------------------------------------------------------------------------------------------------
auto RawPublisher::id() const -> std::uint64_t {
  return impl_->GetTopicId().topic_id.entity_id;
}

}  // namespace grape::ipc
