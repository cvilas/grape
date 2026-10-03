//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/exception.h"

#include <array>
#include <charconv>
#include <concepts>
#include <cstdio>
#include <exception>
#include <iterator>
#include <limits>
#include <span>
#include <string_view>

#include "grape/utils/utils.h"

namespace grape {

namespace {

void write(std::string_view text) noexcept {
  (void)std::fwrite(text.data(), sizeof(char), text.size(), stderr);
}

template <std::integral Integer>
void write(Integer value) noexcept {
  auto buffer = std::array<char, std::numeric_limits<Integer>::digits10 + 3U>{};
  const auto result = std::to_chars(buffer.data(), std::next(buffer.data(), buffer.size()), value);
  if (result.ec == std::errc{}) {
    (void)std::fwrite(buffer.data(), sizeof(char),
                      static_cast<std::size_t>(result.ptr - buffer.data()), stderr);
  }
}

}  // namespace

//-------------------------------------------------------------------------------------------------
Exception::~Exception() = default;

//-------------------------------------------------------------------------------------------------
void Exception::print() noexcept {
  try {
    if (std::current_exception() != nullptr) {
      throw;
    }
  } catch (const grape::Exception& ex) {
    const auto& loc = ex.location();
    const auto loc_fname = utils::truncate(loc.file_name(), "modules");
    write("\nException: ");
    write(ex.what());
    write("\nin\n");
    write(loc.function_name());
    write("\nat\n");
    write(loc_fname);
    write(":");
    write(loc.line());
    write("\nBacktrace:");
    auto idx = 0U;
    for (const auto& trace : ex.trace().trace()) {
      write("\n#");
      write(idx++);
      write(": ");
      write(trace);
    }
  } catch (const std::exception& ex) {
    write("\nException: ");
    write(ex.what());
    write("\n");
  } catch (...) {
    write("\nUnknown exception\n");
  }
}

}  // namespace grape
