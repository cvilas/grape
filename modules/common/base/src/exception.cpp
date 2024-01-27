//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/exception.h"

#include "grape/utils/utils.h"

namespace grape {
Exception::Exception(const std::string& message, std::source_location location)
  : std::runtime_error("[" + std::string(utils::truncate(location.file_name(), "modules")) + ":" +
                       std::to_string(location.line()) + "] " + message) {
}

}  // namespace grape
