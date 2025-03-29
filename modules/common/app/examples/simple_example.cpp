//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include "grape/app/app.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
    grape::app::init(argc, argv, "Simple application example", grape::app::InitFlags::All);
    grape::app::waitForExit();
    return EXIT_SUCCESS;
}