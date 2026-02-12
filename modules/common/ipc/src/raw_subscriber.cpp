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
void raiseMatchEvent(const eCAL::STopicId& topic_id, const eCAL::SSubEventCallbackData& event_data,
                     const grape::ipc::MatchCallback& match_cb) {
  switch (event_data.event_type) {
    case eCAL::eSubscriberEvent::none:
      [[fallthrough]];
    case eCAL::eSubscriberEvent::dropped:
      /* A message was dropped. Choosing to ignore this event! */
      return;
    case eCAL::eSubscriberEvent::connected:
      match_cb({ .remote_entity = { .host = topic_id.topic_id.host_name,
                                    .id = topic_id.topic_id.entity_id },
                 .status = grape::ipc::Match::Status::Matched });
      return;
    case eCAL::eSubscriberEvent::disconnected:
      match_cb({ .remote_entity = { .host = topic_id.topic_id.host_name,
                                    .id = topic_id.topic_id.entity_id },
                 .status = grape::ipc::Match::Status::Unmatched });
      return;
  }
}

//-------------------------------------------------------------------------------------------------
auto createConfig(grape::ipc::QoS qos) -> eCAL::Subscriber::Configuration {
  auto config = eCAL::GetSubscriberConfiguration();
  switch (qos) {
    case grape::ipc::QoS::BestEffort:
      config.layer.shm.enable = true;
      config.layer.udp.enable = true;
      config.layer.tcp.enable = false;
      break;
    case grape::ipc::QoS::Reliable:
      config.layer.shm.enable = true;
      config.layer.udp.enable = false;
      config.layer.tcp.enable = true;
      break;
  }
  return config;
}

}  // namespace

namespace grape::ipc {

struct RawSubscriber::Impl : public eCAL::CSubscriber {
  Impl(const std::string& topic_name, const eCAL::SubEventCallbackT& event_cb,
       const eCAL::Subscriber::Configuration& config)
    : eCAL::CSubscriber(topic_name, eCAL::SDataTypeInformation(), event_cb, config) {
  }
};

//-------------------------------------------------------------------------------------------------
RawSubscriber::RawSubscriber(const std::string& topic, QoS qos,
                             RawSubscriber::DataCallback&& data_cb, MatchCallback&& match_cb) {
  if (not ok()) {
    panic("Not initialised");
  }

  const auto event_cb = [moved_match_cb = std::move(match_cb)](
                            const eCAL::STopicId& topic_id,
                            const eCAL::SSubEventCallbackData& event_data) -> void {
    if (moved_match_cb != nullptr) {
      raiseMatchEvent(topic_id, event_data, moved_match_cb);
    }
  };

  impl_ = std::make_unique<RawSubscriber::Impl>(topic, event_cb, createConfig(qos));

  impl_->SetReceiveCallback([moved_data_cb = std::move(data_cb)](
                                const eCAL::STopicId& id, const eCAL::SDataTypeInformation&,
                                const eCAL::SReceiveCallbackData& data) -> void {
    const auto tp = WallClock::TimePoint(WallClock::Duration{ data.send_timestamp });
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
