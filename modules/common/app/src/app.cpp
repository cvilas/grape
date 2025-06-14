//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/app.h"

#include <atomic>
#include <csignal>
#include <cstring>
#include <exception>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/ipc/session.h"
#include "grape/log/syslog.h"
#include "grape/script/script.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic_flag s_init_flag = ATOMIC_FLAG_INIT;
std::atomic_flag s_exit_flag = ATOMIC_FLAG_INIT;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

//-------------------------------------------------------------------------------------------------
void throwIfUninitialised() {
  if (not s_init_flag.test()) {
    grape::panic<grape::Exception>("Not initialised. Call init() first");
  }
}

//-------------------------------------------------------------------------------------------------
void throwIfInitialised() {
  if (s_init_flag.test()) {
    grape::panic<grape::Exception>("Already initialised");
  }
}

//-------------------------------------------------------------------------------------------------
void handleSignal(int signal) {
  constexpr auto* MSG_PREFIX = "\nReceived signal: ";
  const auto* const signal_name = strsignal(signal);  // NOLINT(concurrency-mt-unsafe)
  std::ignore = write(STDERR_FILENO, MSG_PREFIX, std::strlen(MSG_PREFIX));
  std::ignore = write(STDERR_FILENO, signal_name, std::strlen(signal_name));
  std::ignore = write(STDERR_FILENO, "\n", 1);
  s_exit_flag.test_and_set();
  s_exit_flag.notify_all();
}

//-------------------------------------------------------------------------------------------------
[[noreturn]] void handleTermination() {
  if (std::current_exception() != nullptr) {
    grape::Exception::print();
  } else {
    constexpr auto* MSG_PREFIX = "\nBacktrace\n";
    std::ignore = write(STDERR_FILENO, MSG_PREFIX, strlen(MSG_PREFIX));
    const auto strace = grape::utils::StackTrace::current();
    for (const auto& trace : strace.trace()) {
      std::ignore = write(STDERR_FILENO, trace.c_str(), trace.size());
    }
  }
  _exit(EXIT_FAILURE);
}

//-------------------------------------------------------------------------------------------------
void setExitHandlers() {
  std::ignore = signal(SIGINT, handleSignal);
  std::ignore = signal(SIGTERM, handleSignal);
  std::ignore = std::set_terminate(handleTermination);
}

//-------------------------------------------------------------------------------------------------
void initSyslog(const grape::script::ConfigTable& table) {
  const auto severity_str = table.read<std::string>("severity_threshold")
                                .transform_error([](const auto& err) -> auto {
                                  const auto msg =
                                      std::format("severity_threshold {}", toString(err));
                                  grape::panic<grape::Exception>(msg);
                                  return grape::script::ConfigTable::Error{};  // unreachable
                                })
                                .value();

  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  const auto severity =
      grape::enums::cast<grape::log::Severity>(severity_str)
          .or_else([&severity_str]() -> auto {
            const auto msg = std::format("Invalid severity_threshold={}", severity_str);
            grape::panic<grape::Exception>(msg);
            return std::optional<grape::log::Severity>{ std::nullopt };  // unreachable
          })
          .value();
  // NOLINTEND(bugprone-unchecked-optional-access)

  auto config = grape::log::Config{};
  config.threshold = severity;
  config.logger_name = grape::utils::getProgramName();
  grape::syslog::init(std::move(config));
}

//-------------------------------------------------------------------------------------------------
auto initIpc(const grape::script::ConfigTable& table) {
  const auto scope_str = table.read<std::string>("scope")
                             .transform_error([](const auto& err) -> auto {
                               const auto msg = std::format("IPC scope {}", toString(err));
                               grape::panic<grape::Exception>(msg);
                               return grape::script::ConfigTable::Error{};  // unreachable
                             })
                             .value();

  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  const auto scope =
      grape::enums::cast<grape::ipc::Config::Scope>(scope_str)
          .or_else([&scope_str]() -> std::optional<grape::ipc::Config::Scope> {
            const auto msg = std::format("Invalid IPC scope={}", scope_str);
            grape::panic<grape::Exception>(msg);
            return std::optional<grape::ipc::Config::Scope>{ std::nullopt };  // unreachable
          })
          .value();
  // NOLINTEND(bugprone-unchecked-optional-access)

  auto config = grape::ipc::Config{};
  config.scope = scope;
  config.name = config.name = grape::utils::getProgramName();

  grape::ipc::init(std::move(config));
}

}  // namespace

namespace grape::app {

//-------------------------------------------------------------------------------------------------
void init(int argc, const char** argv, const std::string& description) {
  throwIfInitialised();
  setExitHandlers();

  const auto args =
      grape::conio::ProgramDescription(description)
          .declareOption<std::string>("config", "Application config file path", "config/app.lua")
          .parse(argc, argv)
          .transform_error([](const auto& err) -> auto {
            grape::panic<grape::Exception>(toString(err));
            return conio::ProgramOptions::Error{};  // unreachable
          })
          .value();

  init({ args.getOptionOrThrow<std::string>("config") });
}

//-------------------------------------------------------------------------------------------------
void init(const std::filesystem::path& config_file) {
  throwIfInitialised();
  setExitHandlers();

  // parse config file
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
  initSyslog(log_config.value());
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  syslog::Info("Using configuration file '{}'", maybe_config_path->string());

  // configure IPC
  const auto ipc_config = config_root.read<grape::script::ConfigTable>("ipc");
  if (not ipc_config) {
    panic<Exception>("IPC configuration not found");
  }

  initIpc(ipc_config.value());

  s_init_flag.test_and_set();
}

//-------------------------------------------------------------------------------------------------
auto ok() -> bool {
  throwIfUninitialised();
  return grape::ipc::ok() and not s_exit_flag.test();
}

//-------------------------------------------------------------------------------------------------
void waitForExit() {
  throwIfUninitialised();
  s_exit_flag.wait(false);
}

}  // namespace grape::app
