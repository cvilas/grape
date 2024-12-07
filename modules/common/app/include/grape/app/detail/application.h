//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>

#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/log/logger.h"

namespace grape::app {

///------------------------------------------------------------------------------------------------
/// *NOT USER API*
///
/// This header defines intrinsics required to support grape applications. These are not intended
/// to be user-facing APIs. Instead user applications should use the API from the public header
///------------------------------------------------------------------------------------------------

namespace detail {

//-------------------------------------------------------------------------------------------------
/// Encapsulates common services for GRAPE applications.
class Application {
public:
  void init(const std::filesystem::path& config);
  void cleanup();
  [[nodiscard]] auto createPublisher(const ipc::Topic& topic) -> ipc::Publisher;
  [[nodiscard]] auto createSubscriber(const std::string& topic, ipc::DataCallback&& cb)
      -> ipc::Subscriber;

  [[nodiscard]] auto isInit() const -> bool;
  static auto instance() -> Application&;

  template <typename... Args>
  void log(log::Severity s, const std::source_location& l, std::format_string<Args...> fmt,
           Args&&... args);

private:
  bool is_init_{ false };
  std::unique_ptr<log::Logger> logger_{ nullptr };
  std::unique_ptr<ipc::Session> ipc_session_{ nullptr };
};

//-------------------------------------------------------------------------------------------------
template <typename... Args>
void Application::log(log::Severity s, const std::source_location& l,
                      std::format_string<Args...> fmt, Args&&... args) {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  logger_->log(s, l, fmt, std::forward<Args>(args)...);
}
}  // namespace detail

//-------------------------------------------------------------------------------------------------
/// Logging interface declaration. To understand how this works, look up CTAD and user defined
/// template args deduction guide: https://www.cppstories.com/2021/non-terminal-variadic-args/
template <typename... Args>
struct syslog {
  syslog(log::Severity severity, std::format_string<Args...> fmt, Args&&... args,
         const std::source_location& location = std::source_location::current()) {
    detail::Application::instance().log(severity, location, fmt, std::forward<Args>(args)...);
  }
};

}  // namespace grape::app
