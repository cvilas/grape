//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/utils/stacktrace.h"

#include <array>
#include <memory>

#include <execinfo.h>

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto StackTrace::current() -> StackTrace {
  StackTrace trace;

  auto stack_frames = std::array<void*, MAX_FRAMES + 1>{};
  const auto frame_count = backtrace(stack_frames.data(), MAX_FRAMES);
  const auto symbols = std::unique_ptr<char*, void (*)(void*)>(
      backtrace_symbols(stack_frames.data(), frame_count), std::free);
  trace.symbol_list_.resize(static_cast<std::size_t>(frame_count));
  for (auto i = 1u; i < static_cast<std::size_t>(frame_count); ++i) {
    trace.symbol_list_.at(i - 1) = symbols.get()[i];
  }

  return trace;
}

//-------------------------------------------------------------------------------------------------
auto StackTrace::trace() const -> const std::vector<std::string>& {
  return symbol_list_;
}

}  // namespace grape::utils
