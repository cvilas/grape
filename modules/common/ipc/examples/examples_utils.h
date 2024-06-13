//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/conio/program_options.h"

namespace grape::ipc::ex {

/// Utility function. Reads command line option but throws on error
template <conio::StringStreamable T>
auto getOptionOrThrow(const conio::ProgramOptions& args, const std::string& key) -> T {
  const auto opt = args.getOption<T>(key);
  if (not opt.has_value()) {
    throw conio::ProgramOptions::Error{ opt.error() };
  }
  return opt.value();
}

}  // namespace grape::ipc::ex