//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/utils/stacktrace.h"

#include <array>
#include <memory>
#include <utility>

#include <execinfo.h>

#include "grape/utils/utils.h"

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto StackTrace::current() -> StackTrace {
  StackTrace trace;

  auto stack_frames = std::array<void*, MAX_FRAMES + 1>{};
  const auto frame_count = backtrace(stack_frames.data(), MAX_FRAMES);
  const auto symbols = std::unique_ptr<char*, void (*)(void*)>(
      backtrace_symbols(stack_frames.data(), frame_count), std::free);
  trace.symbol_list_.resize(static_cast<std::size_t>(frame_count));

  for (auto i = 1U; std::cmp_less(i, frame_count); ++i) {
    auto symbol_str = std::string(symbols.get()[i]);
    const auto name_begin = symbol_str.find('(');
    const auto name_end = symbol_str.find('+', name_begin);

    if (name_begin != std::string::npos && name_end != std::string::npos) {
      const auto mangled_name = symbol_str.substr(name_begin + 1, name_end - name_begin - 1);
      const auto demangled = utils::demangle(mangled_name.c_str());
      const auto prefix = symbol_str.substr(0, name_begin + 1);
      const auto suffix = symbol_str.substr(name_end);
      symbol_str = prefix;
      symbol_str += demangled;
      symbol_str += suffix;
    }

    trace.symbol_list_.at(i - 1) = symbol_str;
  }

  return trace;
}

//-------------------------------------------------------------------------------------------------
auto StackTrace::trace() const -> const std::vector<std::string>& {
  return symbol_list_;
}

}  // namespace grape::utils
