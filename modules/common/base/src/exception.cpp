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
    std::ignore = fprintf(stderr, "\n%s\nin\n%s\nat\n%.*s:%u", ex.what(),  //
                          loc.function_name(),                             //
                          static_cast<int>(loc_fname.length()), loc_fname.data(), loc.line());
    std::ignore = fprintf(stderr, "\nBacktrace:");
    auto idx = 0U;
    for (const auto& trace : ex.trace().trace()) {
      std::ignore = fprintf(stderr, "\n#%u: %s", idx++, trace.c_str());
    }
  } catch (const std::exception& ex) {
    std::ignore = fprintf(stderr, "\nException: %s", ex.what());
  } catch (...) {
    std::ignore = fputs("\nUnknown exception", stderr);
  }
  // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

}  // namespace grape
