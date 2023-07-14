//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/exception.h"

#include "grape/utility.h"

namespace grape {
Exception::Exception(const std::string& message, std::source_location location)
  : std::runtime_error(
        "[" + std::string(std::filesystem::relative(location.file_name(), getSourcePath())) + ":" +
        std::to_string(location.line()) + "] " + message) {
}

}  // namespace grape