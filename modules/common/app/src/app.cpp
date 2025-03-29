//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/app.h"

#include <atomic>
#include <csignal>
#include <cstring>
#include <exception>
#include <print>

#include "grape/app/detail/application.h"

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic_flag s_exit_flag = ATOMIC_FLAG_INIT;

void handleSignal(int signal) {
  std::println("\nReceived signal: {}", strsignal(signal));  // NOLINT(concurrency-mt-unsafe)
  s_exit_flag.test_and_set();
  s_exit_flag.notify_all();
}

[[noreturn]] void handleTermination() {
  grape::Exception::print();
  (void)fputs("\nTerminate called\n", stderr);
  (void)fputs("\nBacktrace:\n", stderr);
  auto idx = 0U;
  const auto strace = grape::utils::StackTrace::current();
  for (const auto& trace : strace.trace()) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    (void)fprintf(stderr, "#%u: %s\n", idx++, trace.c_str());
  }
  std::exit(EXIT_FAILURE);  // NOLINT(concurrency-mt-unsafe)
}

}  // namespace

namespace grape::app {

//-------------------------------------------------------------------------------------------------
void init(const std::filesystem::path& config) {
  // setup signal handlers
  std::ignore = signal(SIGINT, handleSignal);
  std::ignore = signal(SIGTERM, handleSignal);

  // setup terminate handler
  std::set_terminate(handleTermination);

  // initialise application
  detail::Application::instance().init(config);
}

//-------------------------------------------------------------------------------------------------
auto ok() -> bool {
  return grape::ipc::ok() and not s_exit_flag.test();
}

//-------------------------------------------------------------------------------------------------
void waitForExit() {
  s_exit_flag.wait(false);
}

}  // namespace grape::app
