//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include "grape/exception.h"
#include "ipc_zenoh.h"  // should be included before zenoh headers

namespace {

//-------------------------------------------------------------------------------------------------
auto transform(const grape::ipc::Session::Config& config) -> zenoh::Config {
  auto zconfig = zenoh::Config::create_default();
  zconfig.insert_json5(Z_CONFIG_MODE_KEY, std::format(R"("{}")", toString(config.mode)));
  const auto is_router_specified = config.router.has_value();
  if (config.mode == grape::ipc::Session::Mode::Client and not is_router_specified) {
    grape::panic<grape::Exception>("'Client' mode requires 'router' to be specified");
  }
  if (is_router_specified) {
    zconfig.insert_json5(Z_CONFIG_CONNECT_KEY, std::format(R"(["{}"])", toString(*config.router)));
  }
  // TODO:
  //- enable timestamp (optional)
  //- enable cache history (optional)
  //- enable shared memory (optional)
  // zconfig.insert_json5(Z_CONFIG_LISTEN_KEY, toString(config.listen_on));
  // todo: insert as ["tcp/[fe80::2145:12c5:9fc3:3c71]:7447", "tcp/192.168.0.2:7447"]
  return zconfig;
}

//-------------------------------------------------------------------------------------------------
auto transform(const std::vector<zenoh::Id>& zids) -> std::vector<grape::ipc::UUID> {
  auto uuids = std::vector<grape::ipc::UUID>{};
  uuids.reserve(zids.size());

  std::ranges::transform(zids, std::back_inserter(uuids),
                         [](const zenoh::Id& z) { return grape::ipc::UUID{ .bytes = z.bytes() }; });
  return uuids;
}

//-------------------------------------------------------------------------------------------------
auto createDataCallback(grape::ipc::DataCallback&& user_cb)
    -> std::function<void(const zenoh::Sample&)> {
  if (user_cb == nullptr) {
    return nullptr;
  }
  return [cb = std::move(user_cb)](const zenoh::Sample& sample) -> void {
    // TODO: Avoid copy. use SpliceIterator instead
    const auto data = sample.get_payload().as_vector();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    cb({ reinterpret_cast<const std::byte*>(data.data()), data.size() });
  };
}
}  // namespace

namespace grape::ipc {

//=================================================================================================
Session::Session(const Session::Config& config) {
  auto zerr = zenoh::ZResult{};
  impl_ = std::make_unique<zenoh::Session>(transform(config),
                                           zenoh::Session::SessionOptions::create_default(), &zerr);
  if (zerr != Z_OK) {
    grape::panic<Exception>(std::format("Failed to create session. Reason: {}", toString(zerr)));
  }
}

//-------------------------------------------------------------------------------------------------
Session::~Session() = default;

//-------------------------------------------------------------------------------------------------
auto Session::id() const -> UUID {
  return { .bytes = impl_->get_zid().bytes() };
}

//-------------------------------------------------------------------------------------------------
auto Session::routers() const -> std::vector<UUID> {
  return transform(impl_->get_routers_z_id());
}

//-------------------------------------------------------------------------------------------------
auto Session::peers() const -> std::vector<UUID> {
  return transform(impl_->get_peers_z_id());
}

//-------------------------------------------------------------------------------------------------
auto Session::createPublisher(const Topic& topic) -> Publisher {
  /// @todo(vilas) Review all the following settings. Expose critical ones
  auto options = zenoh::Session::PublisherOptions::create_default();
  options.congestion_control = Z_CONGESTION_CONTROL_BLOCK;
  options.priority = Z_PRIORITY_DATA;
  options.is_express = false;
  options.reliability = Z_RELIABILITY_BEST_EFFORT;
  auto zerr = zenoh::ZResult{};

  auto pub = Publisher(std::make_unique<zenoh::Publisher>(
      impl_->declare_publisher(topic.key, std::move(options), &zerr)));
  if (zerr != Z_OK) {
    grape::panic<Exception>(std::format("Failed to create publisher. Reason: {}", toString(zerr)));
  }
  return pub;
}

//-------------------------------------------------------------------------------------------------
auto Session::createSubscriber(const std::string& topic, DataCallback&& cb) -> Subscriber {
  auto zerr = zenoh::ZResult{};
  auto sub = Subscriber(std::make_unique<zenoh::Subscriber<void>>(
      impl_->declare_subscriber(topic, createDataCallback(std::move(cb)), zenoh::closures::none,
                                zenoh::Session::SubscriberOptions::create_default(), &zerr)));
  if (zerr != Z_OK) {
    grape::panic<Exception>(std::format("Failed to create subscriber. Reason: {}", toString(zerr)));
  }
  return sub;
}

}  // namespace grape::ipc
