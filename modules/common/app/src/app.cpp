//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/app/app.h"

#include "grape/app/detail/application.h"

namespace grape::app {

//-------------------------------------------------------------------------------------------------
void init(const std::filesystem::path& config) {
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
auto createPublisher(const ipc::Topic& topic) -> ipc::Publisher {
  return detail::Application::instance().createPublisher(topic);
}

//-------------------------------------------------------------------------------------------------
auto createSubscriber(const std::string& topic, ipc::DataCallback&& cb) -> ipc::Subscriber {
  return detail::Application::instance().createSubscriber(topic, std::move(cb));
}

}  // namespace grape::app
