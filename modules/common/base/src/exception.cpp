//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#include "grape/exception.h"

#include <filesystem>

#include "version_impl.h"

namespace grape {
Exception::Exception(const std::string& message, std::source_location location)
  : std::runtime_error("[" +
                       std::string(std::filesystem::relative(location.file_name(), SOURCE_PATH)) +
                       ":" + std::to_string(location.line()) + "] " + message) {
}

}  // namespace grape
