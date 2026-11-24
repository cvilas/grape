//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <utility>

namespace grape {

//=================================================================================================
template <std::invocable ExitFn>
class [[nodiscard]] ScopeGuard {
public:
  explicit ScopeGuard(ExitFn&& exit_fn) : exit_fn_(std::move(exit_fn)) {
  }
  ~ScopeGuard() {
    exit_fn_();
  }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard(ScopeGuard&&) = delete;
  auto operator=(const ScopeGuard&) = delete;
  auto operator=(ScopeGuard&&) = delete;

private:
  ExitFn exit_fn_;
};

template <std::invocable ExitFn>
ScopeGuard(ExitFn&&) -> ScopeGuard<ExitFn>;

}  // namespace grape
