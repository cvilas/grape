//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/exception.h"

#include "grape/utils/utils.h"

namespace grape {

void AbstractException::consume() noexcept {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
  try {
    if (std::current_exception() != nullptr) {
      throw;
    }
  } catch (const AbstractException& ex) {
    const auto& loc = ex.where();
    const auto loc_fname = utils::truncate(loc.file_name(), "modules");
    std::ignore = fprintf(stderr, "\n%s\nin\n%s\nat\n%.*s:%d", ex.what().c_str(),  //
                          loc.function_name(),                                     //
                          static_cast<int>(loc_fname.length()), loc_fname.data(), loc.line());
    std::ignore = fprintf(stderr, "\nBacktrace:");
    for (const auto& s : ex.when().trace()) {
      std::ignore = fprintf(stderr, "\n%s", s.c_str());
    }
  } catch (const std::exception& ex) {
    std::ignore = fprintf(stderr, "\nException: %s", ex.what());
  } catch (...) {
    std::ignore = fputs("\nUnknown exception", stderr);
  }
  // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

}  // namespace grape