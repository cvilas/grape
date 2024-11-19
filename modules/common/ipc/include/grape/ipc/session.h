//=================================================================================================
// Copyright (C) 2024 grape contributors
//=================================================================================================

#pragma once

#include "grape/ipc/common.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/ipc/topic.h"

namespace zenoh {
class Session;
}

namespace grape::ipc {

//=================================================================================================
/// Entry point into IPC API.
///
/// Every process should contain exactly one Session object, which manages all the data publishers
/// and subscribers in the process.
class Session {
public:
  /// Communication modes. See https://zenoh.io/docs/getting-started/deployment/
  enum class Mode { Peer, Client, Router };

  /// Session configuration parameters
  struct Config {
    Session::Mode mode{ Session::Mode::Peer };  //!< Operating mode
    std::optional<Locator> router{};            //!< Router to connect to, if specified
  };

public:
  explicit Session(const Config& config);

  /// @return unique ID of this session
  [[nodiscard]] auto id() const -> UUID;

  /// @return unique IDs of all connected routers
  [[nodiscard]] auto routers() const -> std::vector<UUID>;

  /// @return unique IDs of all connected peers
  [[nodiscard]] auto peers() const -> std::vector<UUID>;

  /// creates a publisher
  /// @param topic topic attributes
  /// @return publisher
  [[nodiscard]] auto createPublisher(const Topic& topic) -> Publisher;

  /// creates a subscriber
  /// @param topic Topic on which to listen to for data from matched publishers
  /// @param cb Data processing callback, triggered on every new sample post on a matching topic
  /// @return subscriber
  [[nodiscard]] auto createSubscriber(const std::string& topic, DataCallback&& cb) -> Subscriber;

  ~Session();
  Session(const Session&) = delete;
  auto operator=(const Session&) = delete;
  Session(Session&&) noexcept = delete;
  auto operator=(Session&&) noexcept = delete;

private:
  std::unique_ptr<zenoh::Session> impl_;
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Session::Mode& mode) -> std::string {
  auto mode_str = std::string(enums::enum_name(mode));
  std::ranges::transform(mode_str, mode_str.begin(), ::tolower);
  return mode_str;
}

}  // namespace grape::ipc
