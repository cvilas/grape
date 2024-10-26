//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include <utility>

#include "grape/exception.h"

namespace {
enum class Error : uint8_t { Bad, RealBad };
constexpr auto toString(const Error& er) -> std::string_view {
  switch (er) {
    case Error::Bad:
      return "Bad";
    case Error::RealBad:
      return "RealBad";
  }
  std::unreachable();
};

using WorkException = grape::Exception<Error>;

void functionThatThrows() {
  grape::panic<WorkException>("Boom!!", Error::RealBad);
}

void doWork() {
  functionThatThrows();
}
}  // namespace

//=================================================================================================
/// Demonstrates usage and handling of exceptions
auto main() -> int {
  try {
    doWork();
    return EXIT_SUCCESS;
  } catch (const WorkException& ex) {  // handle exceptions you care about
    const auto code_str = toString(ex.data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "Error code: %.*s\n",  //
                          static_cast<int>(code_str.length()), code_str.data());
    WorkException::consume();
    return EXIT_FAILURE;
  } catch (...) {  // default handle all other exceptions
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}
