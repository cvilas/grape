//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <fstream>

#include "grape/exception.h"
#include "grape/log/sink.h"

namespace grape::log {

/// Log sink that writes to a file
template <Formatter F>
class FileSink : public Sink {
private:
  std::ofstream fstream_;

public:
  /// Constructs a log sink that writes to the specified file.
  /// @param file_path Path to the file to write to
  /// @param append If true, appends to the file. Otherwise, overwrites it.
  /// @throws Exception if the file cannot be opened
  /// @note The file is opened in append mode by default.
  explicit FileSink(const std::filesystem::path& file_path, bool append = true) {
    const auto mode = std::ofstream::out | (append ? std::ofstream::app : std::ofstream::trunc);
    fstream_.open(file_path, mode);
    if (fstream_.fail()) {
      panic<Exception>(std::format("Unable to open {}", file_path.string()));
    }
    fstream_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
  }

  void write(const Record& rec) {
    fstream_ << F::format(rec) << '\n' << std::flush;
  }
};

}  // namespace grape::log
