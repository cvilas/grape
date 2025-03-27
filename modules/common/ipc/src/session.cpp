//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include <ecal/config/configuration.h>
#include <ecal/core.h>
#include <ecal/pubsub/types.h>

#include "publisher_impl.h"
#include "subscriber_impl.h"

namespace {
//-------------------------------------------------------------------------------------------------
auto toMatchEvent(const eCAL::SPubEventCallbackData& event_data) -> grape::ipc::Match {
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

  return { .status = match_status };
}

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

std::atomic_flag Session::s_instance_exists{ false };

//-------------------------------------------------------------------------------------------------
Session::Session(const Config& config) {
  if (s_instance_exists.test_and_set()) {
    throw std::runtime_error("Only one IPC session is allowed per process");
  }

  auto ecal_config = eCAL::Init::Configuration();
  switch (config.scope) {
    case Config::Scope::Host:
      ecal_config.communication_mode = eCAL::eCommunicationMode::local;
      break;
    case Config::Scope::Network:
      ecal_config.communication_mode = eCAL::eCommunicationMode::network;
      ecal_config.publisher.layer.tcp.enable = true;
      ecal_config.publisher.layer.udp.enable = false;
      ecal_config.subscriber.layer.tcp.enable = true;
      ecal_config.subscriber.layer.udp.enable = false;
      break;
  }
  eCAL::Initialize(ecal_config, config.name);
}

//-------------------------------------------------------------------------------------------------
Session::~Session() {
  eCAL::Finalize();
  s_instance_exists.clear();
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto Session::ok() const -> bool {
  return eCAL::Ok();
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto Session::createPublisher(const Topic& topic, MatchCallback&& match_cb) const -> Publisher {
  const auto event_cb = [moved_match_cb = std::move(match_cb)](
                            const eCAL::STopicId&, const eCAL::SPubEventCallbackData& event_data) {
    if (moved_match_cb != nullptr) {
      moved_match_cb(toMatchEvent(event_data));
    }
  };
  return Publisher(std::make_unique<PublisherImpl>(
      std::make_unique<eCAL::CPublisher>(topic.name, eCAL::SDataTypeInformation(), event_cb)));
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto Session::createSubscriber(const std::string& topic, Subscriber::DataCallback&& data_cb,
                               MatchCallback&& match_cb) const -> Subscriber {
  auto subscriber = std::make_unique<SubscriberImpl>(std::make_unique<eCAL::CSubscriber>(
      topic, eCAL::SDataTypeInformation(),
      [moved_match_cb = std::move(match_cb)](const eCAL::STopicId&,
                                             const eCAL::SSubEventCallbackData& event_data) {
        if (moved_match_cb != nullptr) {
          moved_match_cb(toMatchEvent(event_data));
        }
      }));

  subscriber->sub()->SetReceiveCallback(
      [moved_data_cb = std::move(data_cb)](const eCAL::STopicId&, const eCAL::SDataTypeInformation&,
                                           const eCAL::SReceiveCallbackData& data) {
        const auto tp =
            std::chrono::system_clock::time_point(std::chrono::microseconds(data.send_timestamp));
        if (moved_data_cb != nullptr) {
          moved_data_cb({ .data = { static_cast<const std::byte*>(data.buffer), data.buffer_size },
                          .publish_time = tp });
        }
      });
  return Subscriber(std::move(subscriber));
}

}  // namespace grape::ipc
