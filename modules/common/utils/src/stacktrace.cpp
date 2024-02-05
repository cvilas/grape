//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/utils/stacktrace.h"

#include <array>

#include <execinfo.h>

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto StackTrace::current() -> StackTrace {
  StackTrace trace;
  auto addr_list = std::array<void*, MAX_FRAMES + 1>{};
  const auto depth = backtrace(addr_list.data(), sizeof(addr_list) / sizeof(void*));
  trace.symbol_list_.resize(static_cast<std::size_t>(depth));
  char** symbols = backtrace_symbols(addr_list.data(), depth);
  if (symbols != nullptr) {
    for (int i = 0; i < depth; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      trace.symbol_list_[static_cast<std::size_t>(i)] = symbols[i];
    }
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
    free(static_cast<void*>(symbols));
  }
  return trace;
}

//-------------------------------------------------------------------------------------------------
auto StackTrace::trace() const -> const std::vector<std::string>& {
  return symbol_list_;
}
}  // namespace grape::utils
