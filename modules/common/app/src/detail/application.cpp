//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/detail/application.h"

#include <utility>

#include "grape/log/syslog.h"
#include "grape/script/script.h"

namespace {

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto findConfigFile(const std::filesystem::path& file_name)
    -> std::optional<std::filesystem::path> {
  // try the user-specified path first...
  if (std::filesystem::exists(file_name) && std::filesystem::is_regular_file(file_name)) {
    return file_name;
  }
  // otherwise search for it...
  const auto& search_dirs = grape::utils::getSearchPaths();
  for (const auto& dir : search_dirs) {
    auto full_path = dir / "config" / file_name;
    if (std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path)) {
      return full_path;
    }
  }
  return std::nullopt;
}

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

  grape::syslog::setThreshold(*maybe_severity);  // NOLINT(bugprone-unchecked-optional-access)
}

//-------------------------------------------------------------------------------------------------
auto configureIpc(const grape::script::ConfigTable& table) -> grape::ipc::Session::Config {
  auto config = grape::ipc::Session::Config{};

  // parse IPC scope
  const auto maybe_scope_str = table.read<std::string>("scope");
  if (not maybe_scope_str) {
    grape::panic<grape::Exception>(std::format("IPC scope {}", toString(maybe_scope_str.error())));
  }
  const auto maybe_scope = grape::enums::cast<grape::ipc::Session::Config::Scope>(*maybe_scope_str);
  if (not maybe_scope) {
    const auto msg = std::format("Invalid IPC scope={}", *maybe_scope_str);
    grape::panic<grape::Exception>(msg);
  }
  config.scope = *maybe_scope;  // NOLINT(bugprone-unchecked-optional-access)

  return config;
}

}  // namespace

namespace grape::app::detail {

//-------------------------------------------------------------------------------------------------
auto Application::instance() -> Application& {
  static Application app;
  return app;
}

//-------------------------------------------------------------------------------------------------
auto Application::isInit() const -> bool {
  return is_init_;
}

//-------------------------------------------------------------------------------------------------
auto Application::ok() const -> bool {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  return ipc_session_->ok();
}

//-------------------------------------------------------------------------------------------------
void Application::init(const std::filesystem::path& config_file) {
  if (is_init_) {
    panic<Exception>("Already initialised");
  }
  const auto maybe_config_path = findConfigFile(config_file);
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

  ipc_session_ = std::make_unique<ipc::Session>(configureIpc(*ipc_config));

  is_init_ = true;
}

//-------------------------------------------------------------------------------------------------
void Application::cleanup() {
  is_init_ = false;  // TODO(vilas): race condition with these class variables
  if (ipc_session_ != nullptr) {
    // session_->close();
    ipc_session_.reset();
  }
}

//-------------------------------------------------------------------------------------------------
auto Application::createPublisher(const ipc::Topic& topic, ipc::MatchCallback&& mcb)
    -> ipc::Publisher {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  return ipc_session_->createPublisher(topic, std::move(mcb));
}

//-------------------------------------------------------------------------------------------------
auto Application::createSubscriber(const std::string& topic, ipc::Subscriber::DataCallback&& dcb,
                                   ipc::MatchCallback&& mcb) -> ipc::Subscriber {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  return ipc_session_->createSubscriber(topic, std::move(dcb), std::move(mcb));
}
}  // namespace grape::app::detail
