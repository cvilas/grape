//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/utils/utils.h"

#include <memory>

#include <cxxabi.h>

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto demangle(const char* mangled_name) -> std::string {
  /// \note reference:
  /// https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname

  auto status = -1;
  const auto result = std::unique_ptr<char, void (*)(void*)>{
    abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),  //
    std::free                                                      //
  };
  return (status == 0) ? result.get() : mangled_name;
}

}  // namespace grape::utils
