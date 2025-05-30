//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>

#include "grape/error.h"

//=================================================================================================
auto main() -> int {
  const auto err2 = grape::Error("[mlockall] ", "Unable to lock memory");
  const auto loc = err2.location();
  std::println("[{}:{}({})] : {}", loc.file_name(), loc.line(), loc.function_name(),
               err2.message());
  return EXIT_SUCCESS;
}
