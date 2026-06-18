//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/discovery.h"

#include <set>
#include <tuple>

#include <ecal/pubsub/types.h>
#include <ecal/registration.h>
#include <ecal/types.h>

namespace {
//-------------------------------------------------------------------------------------------------
auto toEndpointInfo(const eCAL::STopicId& id, const eCAL::SDataTypeInformation& type_info)
    -> grape::ipc::EndpointInfo {
  return { .entity_id = { .host = id.topic_id.host_name, .id = id.topic_id.entity_id },
           .type_name = type_info.name,
           .encoding = type_info.encoding };
}
}  // namespace

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
auto discover() -> std::unordered_map<TopicName, Endpoints> {
  auto topics = std::unordered_map<TopicName, Endpoints>{};

  auto pub_ids = std::set<eCAL::STopicId>{};
  std::ignore = eCAL::Registration::GetPublisherIDs(pub_ids);
  for (const auto& id : pub_ids) {
    auto type_info = eCAL::SDataTypeInformation{};
    std::ignore = eCAL::Registration::GetPublisherInfo(id, type_info);
    topics[id.topic_name].publishers.push_back(toEndpointInfo(id, type_info));
  }

  auto sub_ids = std::set<eCAL::STopicId>{};
  std::ignore = eCAL::Registration::GetSubscriberIDs(sub_ids);
  for (const auto& id : sub_ids) {
    auto type_info = eCAL::SDataTypeInformation{};
    std::ignore = eCAL::Registration::GetSubscriberInfo(id, type_info);
    topics[id.topic_name].subscribers.push_back(toEndpointInfo(id, type_info));
  }
  return topics;
}

}  // namespace grape::ipc
