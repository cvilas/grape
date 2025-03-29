//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/app.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  grape::app::init(argc, argv, "Simple application example");
  grape::app::waitForExit();
  return EXIT_SUCCESS;
}
