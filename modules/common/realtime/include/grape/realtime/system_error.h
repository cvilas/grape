//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <string_view>

namespace grape::realtime {

//=================================================================================================
/// Errors from system calls or platform library functions
///
/// @note For list of system error codes, see `errno -l` (On Ubuntu `sudo apt install moreutils`)
struct SystemError {
  int code;  //!< error code (errno) set by failing system call or library function
  std::string_view function_name;  //!< name of failing system call or library function
};

}  // namespace grape::realtime
