//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/ipc2/common.h"
#include "grape/ipc2/publisher.h"
#include "grape/ipc2/subscriber.h"
#include "grape/ipc2/topic.h"

namespace zenoh {
class Session;
}

namespace grape::ipc2 {

//=================================================================================================
/// Entry point into IPC API.
///
/// Every process should contain exactly one Session object, which manages all the data publishers
/// and subscribers in the process.
class Session {
public:
  /// Communication modes. See https://zenoh.io/docs/getting-started/deployment/
  enum class Mode : std::uint8_t {
    Peer,    //!< Connect to every other session directly
    Client,  //!< Connect to other sessions via a router
    Router   //!< Route data between clients and local subnetworks of peers
  };

  /// Session configuration parameters
  struct Config {
    Session::Mode mode{ Session::Mode::Peer };  //!< Operating mode
    std::optional<Locator> router;              //!< Router to connect to
  };

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
[[nodiscard]] constexpr auto toString(const Session::Mode& mode) -> std::string_view {
  return enums::name(mode);
}

}  // namespace grape::ipc2
