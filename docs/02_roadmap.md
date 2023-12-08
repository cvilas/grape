# Roadmap

## Phase 1 - Basics

- :done: Versioning
- :done: Exception definitions
- :done: Command line flags parsing
- :done: Logging library
- :done: Scripting library
- IPC
  - :done: Select a backend
  - port zenoh-c examples
    - Example descriptions: https://github.com/eclipse-zenoh/zenoh/tree/master/examples
    - :done: z_scout
    - :done: z_info
    - :done: z_ping, z_pong (roundtrip)
    - z_pub_thr, z_sub_thr (throughput)
    - z_liveliness
    - z_get_liveliness
    - z_sub_liveliness
    - z_non_blocking_get
    - z_pub_shm
    - z_pub, z_sub
    - z_pull
    - z_put, z_get
    - z_queryable
    - z_delete
    - Document configuration for router, transport, etc. See https://zenoh.io/docs/manual/configuration/
  - zenoh router example  
  - zenoh query example
  - zenoh example pub/sub
    - subscriber prints tx_time, rx_time, latency
    - subscriber prints remote endpoint identity
    - configurable QoS (reliability, priority)
    - notification on match
    - notification on discovery
  - Build wrapper for backend
    - Config
    - Context
    - Publisher
    - Subscriber
    - Router
    - Discovery callback: Uniquely identify pub/sub endpoints
    - Match callback: Uniquely identify matched endpoints
  - Examples
    - simple pub/sub (async message passing) (runnable across subnet)
    - simple query (synchronous message passing)
    - pub/sub/router (runnable across the internet)
    - discovery example
    - throughput benchmark
    - latency benchmark
  - tests
    - discovery
    - match
    - filters
  - Consider hiding backend with FetchContent
- integrate magic_enum
  - refactor existing `toString()` functions
- serdes
  - Choose backend: low overhead, type-safe, fast
  - concept `serialisable`
- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Update CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions

## Phase 2 - Graphics Core

- plot:
  - study: https://pikuma.com/courses/cpp-2d-game-engine-development
  - :done: Choose a backend: implot
  - concept `plottable`
- HW accelerated 3D graphics
  - Select a backend: vsg, ogre, something else
  - Implement a basic scenegraph example and check performance in MacOS and Linux VM
  - Implement scenegraph in our scripting language and have it render by the backend

## Phase 3 - Robotics Core

- timing: periodic timer, watchdog, stopwatch. loop timer
- HW IO
  - canopen
  - joystick
- Video streaming: gstreamer

## Phase 4 - Applications

- Mars Rover (teleop, FPV, visualisation, plots, )

## Phase 5 - Other utilities

- probe library:
  - Port github.com/cvilas/probe library
  - *grape_plant*: closed loop control, deployable on embedded processors.
  - *grape_supervisor*: remote monitoring, graphing, online parameter tuning, and event logging; runs as a separate process, more likely it runs on a different host.
- Realtime: POSIX scheduling wrappers
- utility: hostname, hostaddress, isportinuse, programname, programpath, execute, flag_set
  - enum to string: Copy the general idea from here: <https://godbolt.org/z/6MxYznfbf>
- Behaviour trees
- file cache
- md5sum
- factory using crtp (see scratch)
- FSM
- [ftxui](https://github.com/ArthurSonzogni/FTXUI) based terminal UI apps

## Notes

- Use C++23 or newer features in development

```c++
// hello world in C++23
import std;
auto main() -> int {
  std::println("Hello World!");
  return EXIT_SUCCESS;
}
```

- references:
  - C++20 features: <https://youtu.be/N1gOSgZy7h4>
  - Clean code: <https://youtu.be/9ch7tZN4jeI?si=YkO84hmfQWfq8KO8>
- **Coroutines**: Review usability for async processing, nonblocking IO
- Modules
- monads or_else, and_them, transforms on std::optional: <https://www.cppstories.com/2023/monadic-optional-ops-cpp23/>
- Calendar updates in chrono
- Ranges
- constexpr everywhere
- jthread
- use std::exception for truly exceptional errors, use std::expected everywhere else
- Provide separate interface headers and implementation headers. (See `clean code` video above)
- IoC containers for dependency injection, especially for mocking in tests: <https://github.com/ybainier/Hypodermic>. For why we should use it, see `clean code` video above
- std::stacktrace
