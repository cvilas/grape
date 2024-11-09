//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/app.h"

#include <atomic>
#include <csignal>
#include <cstring>

#include "grape/app/detail/application.h"

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic_flag s_exit_flag = ATOMIC_FLAG_INIT;

void handleSignal(int signal) {
  std::println("\nReceived signal: {}", strsignal(signal));  // NOLINT(concurrency-mt-unsafe)
  s_exit_flag.test_and_set();
  s_exit_flag.notify_all();
}
}  // namespace

namespace grape::app {

//-------------------------------------------------------------------------------------------------
void init(const std::filesystem::path& config) {
  // setup signal handlers
  std::ignore = signal(SIGINT, handleSignal);
  std::ignore = signal(SIGTERM, handleSignal);

  // initialise application
  detail::Application::instance().init(config);
}

//-------------------------------------------------------------------------------------------------
void cleanup() {
  detail::Application::instance().cleanup();
}

//-------------------------------------------------------------------------------------------------
auto isInit() -> bool {
  return detail::Application::instance().isInit();
}

//-------------------------------------------------------------------------------------------------
auto ok() -> bool {
  return detail::Application::instance().ok() and not s_exit_flag.test();
}

//-------------------------------------------------------------------------------------------------
void waitForExit() {
  s_exit_flag.wait(false);
}

//-------------------------------------------------------------------------------------------------
auto createPublisher(const ipc::Topic& topic, ipc::MatchCallback&& mcb) -> ipc::Publisher {
  return detail::Application::instance().createPublisher(topic, std::move(mcb));
}

//-------------------------------------------------------------------------------------------------
auto createSubscriber(const std::string& topic, ipc::Subscriber::DataCallback&& dcb,
                      ipc::MatchCallback&& mcb) -> ipc::Subscriber {
  return detail::Application::instance().createSubscriber(topic, std::move(dcb), std::move(mcb));
}

}  // namespace grape::app
