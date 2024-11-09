//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/app/detail/application.h"

namespace grape::app {

//=================================================================================================
// User API for application-wide services
//=================================================================================================

/// Initialise and configure the application.
/// This should be the first function called in any GRAPE application
/// @param config Top level configuration file
void init(const std::filesystem::path& config);

/// Cleanup resources. Should be called before exiting application
void cleanup();

/// @return true if application is initialised (by calling init())
[[nodiscard]] auto isInit() -> bool;

/// Create a data publisher
  auto createPublisher(const ipc::Topic& topic) -> ipc::Publisher;

  /// Create a data subscriber
  auto createSubscriber(const std::string& topic, ipc::DataCallback&& cb) -> ipc::Subscriber;

/// Application-wide logging interface.
/// @param sev Severity level
/// @param fmt message format string
/// @param args Message args to be formatted
template <typename... Args>
syslog(log::Severity sev, std::format_string<Args...> fmt, Args&&... args) -> syslog<Args...>;

}  // namespace grape::app
