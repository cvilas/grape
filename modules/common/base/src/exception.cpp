//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/exception.h"

#include <exception>

#include "grape/utils/utils.h"

namespace grape {

//-------------------------------------------------------------------------------------------------
void Exception::print() noexcept {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
  try {
    if (std::current_exception() != nullptr) {
      throw;
    }
  } catch (const grape::Exception& ex) {
    const auto& loc = ex.location();
    const auto loc_fname = utils::truncate(loc.file_name(), "modules");
    (void)fprintf(stderr, "\nException: %s\nin\n%s\nat\n%.*s:%u", ex.what(), loc.function_name(),
                  static_cast<int>(loc_fname.length()), loc_fname.data(), loc.line());
    (void)fprintf(stderr, "\nBacktrace:");
    auto idx = 0U;
    for (const auto& trace : ex.trace().trace()) {
      (void)fprintf(stderr, "\n#%u: %s", idx++, trace.c_str());
    }
  } catch (const std::exception& ex) {
    (void)fprintf(stderr, "\nException: %s\n", ex.what());
  } catch (...) {
    (void)fputs("\nUnknown exception\n", stderr);
  }
  // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

}  // namespace grape
