//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/exception.h"
#include "grape/log/syslog.h"

//=================================================================================================
auto main(int /*argc*/, const char** /*argv[]*/) -> int {
  try {
    grape::syslog::setThreshold(grape::log::Severity::Debug);
    grape::syslog::Log(grape::log::Severity::Critical, "A critical error message");
    grape::syslog::Log(grape::log::Severity::Error, "An error message");
    grape::syslog::Log(grape::log::Severity::Warn, "A warning message");
    grape::syslog::Log(grape::log::Severity::Note, "message='{}'", "A note message");
    grape::syslog::Log(grape::log::Severity::Info, "An informational message");
    grape::syslog::Log(grape::log::Severity::Debug, "A debug message");
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
