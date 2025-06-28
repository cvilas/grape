//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/publisher.h"

#include <chrono>

#include <ecal/pubsub/publisher.h>
#include <ecal/pubsub/types.h>

#include "grape/exception.h"
#include "grape/ipc/session.h"

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

struct Publisher::Impl : public eCAL::CPublisher {
  Impl(const std::string& topic_name, const eCAL::PubEventCallbackT& event_cb)
    : eCAL::CPublisher(topic_name, eCAL::SDataTypeInformation(), event_cb) {
  }
};

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(const std::string& topic, MatchCallback&& match_cb) {
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
  impl_ = std::make_unique<Publisher::Impl>(topic, event_cb);
}

//-------------------------------------------------------------------------------------------------
Publisher::~Publisher() = default;

//-------------------------------------------------------------------------------------------------
Publisher::Publisher(Publisher&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
void Publisher::publish(std::span<const std::byte> bytes) const {
  const auto now = std::chrono::system_clock::now();
  const auto us =
      std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
  std::ignore = impl_->Send(bytes.data(), bytes.size(), us);
}

//-------------------------------------------------------------------------------------------------
auto Publisher::id() const -> std::uint64_t {
  return impl_->GetTopicId().topic_id.entity_id;
}

}  // namespace grape::ipc
