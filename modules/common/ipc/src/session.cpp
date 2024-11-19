//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include "grape/ipc/subscriber.h"
#include "ipc_zenoh.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto transform(const grape::ipc::Session::Config& config) -> zenoh::Config {
  auto zconfig = zenoh::Config::create_default();
  (void)config;
  zconfig.insert_json5(Z_CONFIG_MODE_KEY, std::format("\"{}\"", toString(config.mode)));
  // TODO:
  // - enable timestamp (optional)
  // - enable cache history (optional)
  // - enable shared memory (optional)
  // if (config.router.has_value()) {
  //   zconfig.insert_json5(Z_CONFIG_CONNECT_KEY, toString(config.router.value()));
  // }
  //  zconfig.insert_json5(Z_CONFIG_LISTEN_KEY, toString(config.listen_on));
  //  todo: insert as ["tcp/[fe80::2145:12c5:9fc3:3c71]:7447", "tcp/192.168.0.2:7447"]
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
Session::Session(const Session::Config& config)
  : impl_(std::make_unique<zenoh::Session>(transform(config))) {
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
  return Publisher(std::make_unique<zenoh::Publisher>(impl_->declare_publisher(topic.key)));
}

//-------------------------------------------------------------------------------------------------
auto Session::createSubscriber(const std::string& topic, DataCallback&& cb) -> Subscriber {
  return Subscriber(std::make_unique<zenoh::Subscriber<void>>(
      impl_->declare_subscriber(topic, createDataCallback(std::move(cb)), zenoh::closures::none)));
}

}  // namespace grape::ipc
