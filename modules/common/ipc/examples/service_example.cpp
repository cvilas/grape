//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <thread>

#include <ecal/core.h>
#include <ecal/service/client.h>
#include <ecal/service/server.h>
#include <ecal/service/types.h>

#include "grape/exception.h"

//=================================================================================================
auto main() -> int {
  try {
    eCAL::Initialize("service_example");

    static constexpr auto SERVICE_NAME = "hello_world_service";
    static constexpr auto METHOD_NAME = "hello";
    static constexpr auto METHOD_PARAM = "get";

    auto server_event_cb = [](const eCAL::SServiceId& service_id,
                              const eCAL::SServerEventCallbackData& event) -> void {
      std::println("[Server] {}: {} {}", service_id.service_name, service_id.service_id.host_name,
                   to_string(event.type));
    };

    // Create a service
    auto server = eCAL::CServiceServer(SERVICE_NAME, server_event_cb);

    // Define a service method and the callback to invoke when the method is requested
    const auto method =
        eCAL::SServiceMethodInformation{ .method_name = METHOD_NAME,
                                         .request_type = eCAL::SDataTypeInformation{},
                                         .response_type = eCAL::SDataTypeInformation{} };

    const auto service_cb = [](const eCAL::SServiceMethodInformation& method_info,
                               const std::string& request, std::string& response) -> int {
      std::println("[Server] Received request {}?{}", method_info.method_name, request);
      response = "Hello, World!";
      return 0;
    };

    server.SetMethodCallback(method, service_cb);

    // Create client
    auto service_method_info_set = std::set<eCAL::SServiceMethodInformation>{ method };
    auto client_event_cb = [](const eCAL::SServiceId& service_id,
                              const eCAL::SClientEventCallbackData& event) -> void {
      std::println("[Client] '{}' from '{}' {}", service_id.service_name,
                   service_id.service_id.host_name, to_string(event.type));
    };

    auto client = eCAL::CServiceClient(SERVICE_NAME, service_method_info_set, client_event_cb);
    static constexpr auto SERVICE_WAIT = std::chrono::milliseconds(1000);
    while (!client.IsConnected()) {
      std::println("[Client] Waiting for a service ..");
      std::this_thread::sleep_for(SERVICE_WAIT);
    }

    // Call the service and wait for the response
    auto responses = eCAL::ServiceResponseVecT{};
    while (eCAL::Ok()) {
      if (not client.CallWithResponse(METHOD_NAME, METHOD_PARAM, responses, SERVICE_WAIT.count())) {
        std::println("[Client] Failed to call service");
        return EXIT_FAILURE;
      }
      std::println("[Client] Received {} responses", responses.size());
      for (const auto& resp : responses) {
        std::println("[Client] Response: {}", resp.response);
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    eCAL::Finalize();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
