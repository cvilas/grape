//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>

#include "grape/exception.h"
#include "grape/ipc/session.h"

namespace grape::app::detail {

///------------------------------------------------------------------------------------------------
/// *NOT USER API*
///
/// This header defines intrinsics required to support grape applications. These are not intended
/// to be user-facing APIs. Instead user applications should use the API from the public header
///------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// Encapsulates common services for GRAPE applications.
class Application {
public:
  void init(const std::filesystem::path& config);
  [[nodiscard]] auto isInit() const -> bool;
  [[nodiscard]] auto ok() const -> bool;
  void cleanup();
  static auto instance() -> Application&;

private:
  bool is_init_{ false };
};

}  // namespace grape::app::detail
