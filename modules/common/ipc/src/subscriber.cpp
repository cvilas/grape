//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/subscriber.h"

#include <ecal/pubsub/subscriber.h>
#include <ecal/pubsub/types.h>

#include "grape/exception.h"
#include "grape/ipc/session.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto toMatchEvent(const eCAL::SSubEventCallbackData& event_data) -> grape::ipc::Match {
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
  return { .status = match_status };
}
}  // namespace

namespace grape::ipc {

struct Subscriber::Impl : public eCAL::CSubscriber {
  Impl(const std::string& topic_name, const eCAL::SubEventCallbackT& event_cb)
    : eCAL::CSubscriber(topic_name, eCAL::SDataTypeInformation(), event_cb) {
  }
};

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(const std::string& topic, Subscriber::DataCallback&& data_cb,
                       MatchCallback&& match_cb) {
  if (not ok()) {
    panic<Exception>("Not initialised");
  }

  const auto event_cb = [moved_match_cb = std::move(match_cb)](
                            const eCAL::STopicId&,
                            const eCAL::SSubEventCallbackData& event_data) -> void {
    if (moved_match_cb != nullptr) {
      moved_match_cb(toMatchEvent(event_data));
    }
  };

  impl_ = std::make_unique<Subscriber::Impl>(topic, event_cb);

  impl_->SetReceiveCallback(
      [moved_data_cb = std::move(data_cb)](const eCAL::STopicId&, const eCAL::SDataTypeInformation&,
                                           const eCAL::SReceiveCallbackData& data) -> void {
        const auto tp =
            std::chrono::system_clock::time_point(std::chrono::microseconds(data.send_timestamp));
        if (moved_data_cb != nullptr) {
          moved_data_cb({ .data = { static_cast<const std::byte*>(data.buffer), data.buffer_size },
                          .publish_time = tp });
        }
      });
}

//-------------------------------------------------------------------------------------------------
Subscriber::~Subscriber() = default;

//-------------------------------------------------------------------------------------------------
Subscriber::Subscriber(Subscriber&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
auto Subscriber::getPublisherCount() const -> std::size_t {
  return impl_->GetPublisherCount();
}
}  // namespace grape::ipc
