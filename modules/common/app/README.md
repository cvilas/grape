# README: app

## Brief

GRAPE application services

## TODO

- :done: Implement enum_to_string and string_to_enum.
- :done: Implement config file handling
- :done: Integrate configuration processing for logger
- :done: Implement configuration file search order following [Linux File System rules](https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard)
- :done: Document configuration file search order
- :done: Reduce verbosity in command line args handling
- :done: Implement zenoh session
- :done: Define `listen_on` and `connect_to` for routers
- Ability to create and destroy publishers and subscribers at will (required for teleop mux)
- Implement basic ipc configuration processing.
- Fix unit tests
  - failure happens in `toString(const zenoh::ZResult&)`, perhaps in unordered map
  - further the error happens due to network failure. So test needs to run with a router and/or custom config
- PoC IPC experiments
  - Case 1: pub-peer on PC1, sub-peer on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 2: pub-peer + router on PC1, sub-peer + router on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 3: Extend case2 by adding a PC4 with router and sub-client. Confirm sub-client on PC4 receives data from pub-peer on PC1.
- Implement advanced ipc configuration processing. 
  - Copy and load [default configuration](https://github.com/eclipse-zenoh/zenoh/blob/main/DEFAULT_CONFIG.json5) 
  - Select parameters to explicitly override with hardcoded options
  - Select parameters to override with user-selected options
  - See other project for reference on which fields to set
- Implement lua binding for certain useful grape utilities 
  - set `ipc{topic_prefix=grape::utils::hostName()}` by calling lua binding
- Implement example program for applications communicating with each other
- Reduce verbosity in signal handling
  - `grape::app::loopOnce()` (needs better name to clarify this checks for exceptions and exit conditions once)
  - `grape::app::waitForExit()` (needs better name to clarify this blocks waiting for exit condition)
- Write a [document](../../../docs/04_architecture.md) explaining how to write grape applications
  - Implement libraries without dependency on IPC 
  - Follow the original OOP concept from Smalltalk. 'Objects' are services that respond to 'messages'
  - Implement those objects using facilities from `grape::app` for configuration file handling, logging and IPC
  - Objects run as processes distributed across CPUs and communicate with each other over IPC

  An example grape application that publishes on a topic
  
  ```C++
  #include <cstdlib>
  #include <chrono>
  #include <print>
  #include <thread>

  #include "grape/app/app.h"

  auto main(int argc, const char* argv[]) -> int {
    try{
      grape::app::init(argc, argv);

      static constexpr auto topic = "hello";
      auto pub = grape::app::Publisher(topic);

      auto count = 0u;
      static constexpr auto LOOP_SLEEP = std::chrono::milliseconds(100);
      while(grape::app::ok()){
        std::this_thread::sleep_for(LOOP_SLEEP);

        auto msg = std::format("Hello World {}", ++count);
        grape::app::syslog(grape::log::Severity::Info, "{}", msg);
        pub.publish(msg);
      }
      return EXIT_SUCCESS;
    } catch(...) {
      grape::Exception::print();
      return EXIT_FAILURE;
    }
  }
  ```

  An example grape application that subscribes to the topic above
  
  ```C++
  #include <cstdlib>
  #include <chrono>
  #include <print>
  #include <thread>

  #include "grape/app/app.h"

  auto main(int argc, const char* argv[]) -> int {
    try{
      grape::app::init(argc, argv);

      static constexpr auto topic = "hello";
      auto callback = [](std::span<const std::byte>& bytes) {
        const auto msg = std::string(static_cast<const char*>(bytes.data()), bytes.size_bytes());
        std::println("{}", msg);
      };
      auto sub = grape::app::Subscriber(topic, std::move(callback));

      static constexpr auto LOOP_SLEEP = std::chrono::milliseconds(100);
      while(grape::app::ok()){
        std::this_thread::sleep_for(LOOP_SLEEP);
      }
      return EXIT_SUCCESS;
    } catch(...) {
      grape::Exception::print();
      return EXIT_FAILURE;
    }
  }
  ```

  Some details will change for the sake of flexibility. For instance
  - `grape::app::ok()` may be replaced by a standard signal handler or barrier mechanism to keep process running until exit condition
  - Additional steps may be introduced for data serialisation and deserialisation
