//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>

#include "grape/ipc/match.h"
#include "grape/ipc/publisher.h"
#include "grape/ipc/subscriber.h"
#include "grape/ipc/topic.h"
#include "grape/utils/utils.h"

namespace grape::ipc {

//=================================================================================================
/// Entry point into IPC API.
///
/// - Enables message passing across processes on host and entities distributed across a LAN.
/// - Manages data publishers and subscribers.
/// - Every process should contain exactly one Session object
class Session {
public:
  /// Session configuration parameters
  struct Config {
    /// Operating scope of publishers and subscribers in the session
    enum class Scope : std::uint8_t {
      Host,    //!< Messages confined to host
      Network  //!< Messages can be exchanged across LAN
    };
    std::string name = utils::getProgramName();  //!< user-defined identifier
    Scope scope = Scope::Host;
  };

  explicit Session(const Config& config);

  /// @return true if session state is nominal and error-free
  [[nodiscard]] auto ok() const -> bool;

  /// creates a publisher
  /// @param topic topic attributes
  /// @param match_cb Match callback, triggered on matched/unmatched with a remote subscriber
  /// @return publisher
  [[nodiscard]] auto createPublisher(const Topic& topic, MatchCallback&& match_cb = nullptr) const
      -> Publisher;

  /// creates a subscriber
  /// @param topic Topic on which to listen to for data from matched publishers
  /// @param data_cb Data processing callback, triggered on every newly received data sample
  /// @param match_cb Match callback, triggered when matched/unmatched with a remote publisher
  /// @return subscriber
  [[nodiscard]] auto createSubscriber(const std::string& topic, Subscriber::DataCallback&& data_cb,
                                      MatchCallback&& match_cb = nullptr) const -> Subscriber;

  ~Session();
  Session(const Session&) = delete;
  auto operator=(const Session&) = delete;
  Session(Session&&) noexcept = delete;
  auto operator=(Session&&) noexcept = delete;

private:
  static std::atomic_flag s_instance_exists;
};
}  // namespace grape::ipc
