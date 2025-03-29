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

/// @return true if application should continue executing. False indicates that the node should exit
[[nodiscard]] auto ok() -> bool;

/// Wait for signal to exit application
void waitForExit();

/// Create a data publisher
auto createPublisher(const ipc::Topic& topic, ipc::MatchCallback&& mcb = nullptr) -> ipc::Publisher;

/// Create a data subscriber
auto createSubscriber(const std::string& topic, ipc::Subscriber::DataCallback&& dcb,
                      ipc::MatchCallback&& mcb = nullptr) -> ipc::Subscriber;

}  // namespace grape::app
