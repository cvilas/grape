//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/exception.h"
#include "grape/log/syslog.h"

//=================================================================================================
auto main(int /*argc*/, const char** /*argv[]*/) -> int {
  try {
    auto config = grape::log::Config{};
    config.threshold = grape::log::Severity::Debug;
    grape::syslog::init(std::move(config));
    grape::syslog::Critical("A critical error message");
    grape::syslog::Error("An error message");
    grape::syslog::Warn("A warning message");
    grape::syslog::Note("message='{}'", "A note message");
    grape::syslog::Info("An informational message");
    grape::syslog::Debug("A debug message");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
