//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>

#include "grape/log/logger.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  grape::log::Logger logger({});
  logger.log(grape::log::Severity::Error, "A log message");
  return EXIT_SUCCESS;
}
