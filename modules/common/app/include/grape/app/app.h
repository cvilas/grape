//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/app/detail/application.h"

namespace grape::app {

//=================================================================================================
// User API for application-wide services
//=================================================================================================

// void init(int argc, const char* argv[], const std::string& description =
// utils::getProgramName());

/// Initialise and configure the application.
/// This should be the first function called in any GRAPE application
/// @param config Top level configuration file
void init(const std::filesystem::path& config);

/// @return true if application should continue executing. False indicates that the node should exit
[[nodiscard]] auto ok() -> bool;

/// Wait for signal to exit application
void waitForExit();

}  // namespace grape::app
