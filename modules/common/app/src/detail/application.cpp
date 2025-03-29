//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/detail/application.h"

#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/script/script.h"

namespace {

//-------------------------------------------------------------------------------------------------
void configureLogger(const grape::script::ConfigTable& table) {
  const auto maybe_severity_str = table.read<std::string>("severity_threshold");
  if (not maybe_severity_str) {
    grape::panic<grape::Exception>(
        std::format("severity_threshold {}", toString(maybe_severity_str.error())));
  }
  const auto maybe_severity = grape::enums::cast<grape::log::Severity>(*maybe_severity_str);
  if (not maybe_severity) {
    const auto msg = std::format("Invalid severity_threshold={}", *maybe_severity_str);
    grape::panic<grape::Exception>(msg);
  }
  auto config = grape::log::Config{};
  config.threshold = maybe_severity.value();  // NOLINT(bugprone-unchecked-optional-access)
  config.logger_name = grape::utils::getProgramName();
  grape::syslog::init(std::move(config));
}

//-------------------------------------------------------------------------------------------------
auto configureIpc(const grape::script::ConfigTable& table) {
  auto config = grape::ipc::Config{};

  // parse IPC scope
  const auto maybe_scope_str = table.read<std::string>("scope");
  if (not maybe_scope_str) {
    grape::panic<grape::Exception>(std::format("IPC scope {}", toString(maybe_scope_str.error())));
  }
  const auto maybe_scope = grape::enums::cast<grape::ipc::Config::Scope>(*maybe_scope_str);
  if (not maybe_scope) {
    const auto msg = std::format("Invalid IPC scope={}", *maybe_scope_str);
    grape::panic<grape::Exception>(msg);
  }
  config.scope = maybe_scope.value();  // NOLINT(bugprone-unchecked-optional-access)
  config.name = config.name = grape::utils::getProgramName();

  grape::ipc::init(std::move(config));
}

}  // namespace

namespace grape::app::detail {

//-------------------------------------------------------------------------------------------------
auto Application::instance() -> Application& {
  static Application app;
  return app;
}

//-------------------------------------------------------------------------------------------------
auto Application::ok() const -> bool {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  return grape::ipc::ok();
}

//-------------------------------------------------------------------------------------------------
void Application::init(const std::filesystem::path& config_file) {
  if (is_init_) {
    panic<Exception>("Already initialised");
  }
  const auto maybe_config_path = utils::resolveFilePath(config_file);
  if (not maybe_config_path) {
    panic<Exception>(std::format("Config file '{}' not found", config_file.string()));
  }
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  const auto config_script = script::ConfigScript(*maybe_config_path);
  const auto config_root = config_script.table();

  // configure logger first
  const auto log_config = config_root.read<grape::script::ConfigTable>("logger");
  if (not log_config) {
    panic<Exception>("Logger configuration not found");
  }
  configureLogger(log_config.value());
  syslog::Log(grape::log::Severity::Info, "Using configuration file '{}'",
              maybe_config_path->string());  // NOLINT(bugprone-unchecked-optional-access)

  // configure IPC
  const auto ipc_config = config_root.read<grape::script::ConfigTable>("ipc");
  if (not ipc_config) {
    panic<Exception>("IPC configuration not found");
  }

  configureIpc(ipc_config.value());

  is_init_ = true;
}

}  // namespace grape::app::detail
