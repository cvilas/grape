//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/detail/application.h"

#include "grape/script/script.h"
#include "grape/utils/utils.h"

namespace {

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto findConfigFile(const std::filesystem::path& file_name)
    -> std::optional<std::filesystem::path> {
  // try the user-specified path first...
  if (std::filesystem::exists(file_name) && std::filesystem::is_regular_file(file_name)) {
    return file_name;
  }
  // otherwise search for it...
  const auto& search_dirs = grape::utils::getSearchDirs();
  for (const auto& dir : search_dirs) {
    auto full_path = dir / "config" / file_name;
    if (std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path)) {
      return full_path;
    }
  }
  return std::nullopt;
}

//-------------------------------------------------------------------------------------------------
auto configureLogger(const grape::script::ConfigTable& table) -> grape::log::Config {
  const auto app_name = grape::utils::getProgramPath().filename().string();
  auto config = grape::log::Config{ .logger_name = app_name };
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

  config.threshold = *maybe_severity;  // NOLINT(bugprone-unchecked-optional-access)

  return config;
}

//-------------------------------------------------------------------------------------------------
auto configureIpc(const grape::script::ConfigTable& table) -> grape::ipc::Session::Config {
  auto config = grape::ipc::Session::Config{};

  // parse mode
  const auto maybe_mode_str = table.read<std::string>("mode");
  if (not maybe_mode_str) {
    grape::panic<grape::Exception>(std::format("mode {}", toString(maybe_mode_str.error())));
  }
  const auto maybe_mode = grape::enums::cast<grape::ipc::Session::Mode>(*maybe_mode_str);
  if (not maybe_mode) {
    const auto msg = std::format("Invalid mode={}", *maybe_mode_str);
    grape::panic<grape::Exception>(msg);
  }
  config.mode = *maybe_mode;  // NOLINT(bugprone-unchecked-optional-access)

  // parse router
  // TODO(vilas): Decide whether router should always be specified or not. If so, throw error if not
  // specified
  const auto maybe_router_str = table.read<std::string>("router");
  if (maybe_router_str.has_value() and not maybe_router_str.value().empty()) {
    config.router = grape::ipc::Locator::fromString(*maybe_router_str);
    if (not config.router) {
      const auto msg = std::format("Invalid router={}", *maybe_router_str);
      grape::panic<grape::Exception>(msg);
    }
  }

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
  logger_ = std::make_unique<log::Logger>(configureLogger(*log_config));
  grape::log::Log(*logger_, grape::log::Severity::Info, "Using configuration file '{}'",
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
auto Application::createPublisher(const ipc::Topic& topic) -> ipc::Publisher {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  return ipc_session_->createPublisher(topic);
}

//-------------------------------------------------------------------------------------------------
auto Application::createSubscriber(const std::string& topic, ipc::DataCallback&& cb)
    -> ipc::Subscriber {
  if (not is_init_) {
    panic<Exception>("Not initialised. Call init() first");
  }
  return ipc_session_->createSubscriber(topic, std::move(cb));
}
}  // namespace grape::app::detail