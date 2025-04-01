//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>

#include "grape/log/record.h"

namespace grape::log {

//=================================================================================================
/// Interface for log record text formatters, for use with log sinks
template <typename T>
concept Formatter = requires(const Record& record) {
  { T::format(record) } -> std::same_as<std::string>;
};

//=================================================================================================
/// Abstract interface for log sinks
struct Sink {
  virtual ~Sink() = default;
  virtual void write(const Record& record) = 0;

  Sink(Sink const&) = delete;
  Sink(Sink&&) = default;
  auto operator=(Sink const&) = delete;
  auto operator=(Sink&&) -> Sink& = default;

protected:
  Sink() = default;
};
}  // namespace grape::log
