//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/raw_publisher.h"

#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/types.h>

#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/wall_clock.h"

namespace {

//-------------------------------------------------------------------------------------------------
void raiseMatchEvent(const eCAL::STopicId& topic_id, const eCAL::SPubEventCallbackData& event_data,
                     const grape::ipc::MatchCallback& match_cb) {
  switch (event_data.event_type) {
    case eCAL::ePublisherEvent::none:
      [[fallthrough]];
    case eCAL::ePublisherEvent::dropped:
      /* some subscribers missed one or more messages. Choosing to ignore this event! */
      return;
    case eCAL::ePublisherEvent::connected:
      match_cb({ .remote_entity = { .host = topic_id.topic_id.host_name,
                                    .id = topic_id.topic_id.entity_id },
                 .status = grape::ipc::Match::Status::Matched });
      return;
    case eCAL::ePublisherEvent::disconnected:
      match_cb({ .remote_entity = { .host = topic_id.topic_id.host_name,
                                    .id = topic_id.topic_id.entity_id },
                 .status = grape::ipc::Match::Status::Unmatched });
      return;
  }
}

//-------------------------------------------------------------------------------------------------
auto createConfig() -> eCAL::Publisher::Configuration {
  auto config = eCAL::GetPublisherConfiguration();
  config.layer.shm.enable = true;
  config.layer.udp.enable = true;
  config.layer.tcp.enable = true;
  return config;
}

}  // namespace

namespace grape::ipc {

struct RawPublisher::Impl : public eCAL::CPublisher {
  Impl(const std::string& topic_name, const eCAL::PubEventCallbackT& event_cb,
       const eCAL::Publisher::Configuration& config)
    : eCAL::CPublisher(topic_name, eCAL::SDataTypeInformation(), event_cb, config) {
  }
};

//-------------------------------------------------------------------------------------------------
RawPublisher::RawPublisher(const std::string& topic, MatchCallback&& match_cb) {
  if (not ok()) {
    panic("Not initialised");
  }
  const auto event_cb = [moved_match_cb = std::move(match_cb)](
                            const eCAL::STopicId& topic_id,
                            const eCAL::SPubEventCallbackData& event_data) -> void {
    if (moved_match_cb != nullptr) {
      raiseMatchEvent(topic_id, event_data, moved_match_cb);
    }
  };
  impl_ = std::make_unique<RawPublisher::Impl>(topic, event_cb, createConfig());
}

//-------------------------------------------------------------------------------------------------
RawPublisher::~RawPublisher() = default;

//-------------------------------------------------------------------------------------------------
RawPublisher::RawPublisher(RawPublisher&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
auto RawPublisher::publish(std::span<const std::byte> bytes) const -> std::expected<void, Error> {
  const auto now = WallClock::now().time_since_epoch().count();
  if (not impl_->Send(bytes.data(), bytes.size(), now)) {
    if (impl_->GetSubscriberCount() > 0U) {
      return std::unexpected{ Error::PublishFailed };
    }
  }
  return {};
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
