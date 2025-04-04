//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/utils/utils.h"

#include <cstdlib>
#include <memory>

#include <cxxabi.h>

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto demangle(const char* mangled_name) -> std::string {
  auto status = -1;
  const auto result = std::unique_ptr<char, void (*)(void*)>{
    abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),  //
    std::free                                                      //
  };
  return { (status == 0) ? result.get() : mangled_name };
}

}  // namespace grape::utils
