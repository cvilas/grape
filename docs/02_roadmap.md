# Roadmap

## Phase 1 - Base

- :done: Versioning
- :done: Exception definitions
- :done: Command line flags parsing
- :done: Logging library
- :done: Scripting library
- :done: Message passing
- Realtime services
  - :done: non-blocking mpsc queue
  - :done: memory locking function
  - :done: priority inheriting mutex
  - :done: Scheduler functions
  - Periodic timer, watchdog, stopwatch
  - RT thread
    - implementation
    - documentation recommending how to implement RT program
  - Benchmark program to print mean and variance in timer accuracy
- Refactor Exceptions ([inspiration](https://youtu.be/Oy-VTqz1_58)) 
  - Single exception class and no hierarchy
  - Explain recommended usage and rationale in the docs

## Phase 2 - Demo application 1 - GRAPE

- Design
  - :done: Start requirement document
  - Sketch out the architecture: in RT path no memory allocation, minimal network overheads, no system calls, no locks
- Study: 
  - [cactus-rt](https://github.com/cactusdynamics/cactus-rt)
  - [wesplot](https://github.com/cactusdynamics/wesplot)
  - [DataTamer](https://github.com/PickNikRobotics/data_tamer) and [probe](github.com/cvilas/probe)
  - Sign up for [game engine](https://pikuma.com/courses/cpp-2d-game-engine-development) course
  - Document all these resources
- Implement:
  - Implement Plant
  - Implement Monitor 
- plot:
  - :done: Choose a backend: implot (docking branch)
  - concept `plottable`
- serdes
  - Choose backend: low overhead, type-safe, fast, no external dependencies, supports C++ and Python
  - concept `serialisable`
- zenoh video capture/display: https://github.com/eclipse-zenoh/zenoh-demos/tree/master/computer-vision/zcam
- Consider integrating mp_uints library

## Phase 3 - CI

- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions
- study
  - [reflect-cpp](https://github.com/getml/reflect-cpp)

## Phase 4 - Audio, Video and 3D Graphics

- Audio/Video streaming:
  - Choose backend for audio/video device handling and stream processing
  - Implement basic examples for AV capture, streaming and display

- HW accelerated 3D graphics
  - Select a backend: vsg, ogre, raylib, something else
  - Implement a basic scenegraph example and check performance in MacOS and Linux VM
  - Implement scenegraph in our scripting language and have it render by the backend

## Phase 5 - Robotics Core

- enum-to-string and string-to-enum
  - enum to string: Copy the general idea from here: <https://godbolt.org/z/6MxYznfbf>
  - consider magic_enum and other options and choose one
  - refactor existing `toString()` functions
- Additional zenoh examples
  - zenoh_pub_cache: Understand what it is supposed to do and make it work
  - zenoh_query_sub - make it work
  - z_storage
  - any new examples that have appeared in zenoh-c, zenoh-cpp
  - describe and provide example for user attachments. z_get/pub/queryable_attachment
    - The Attachment feature is a generic way for users to attach any "metadata" to the payload they send through Zenoh. The same could be achieved by taking your data and metadata, serialize them all in a single payload and send it through Zenoh. The key benefit here is that if your main payload and metadata come from different memory regions, it avoids allocating a new buffer and serializing thus copying your payload and metadata in this new buffer. In a way it brings similar benefits as Vectored I/O and writev function: https://en.wikipedia.org/wiki/Vectored_I/O
- HW IO
  - CANopen
  - joystick
- Behaviour trees: Consider building from first principles
- FSM: introspectable, visualise state transition graph using graphviz.
- PoC IPC experiments
  - Case 1: pub-peer on PC1, sub-peer on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 2: pub-peer + router on PC1, sub-peer + router on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 3: Extend case2 by adding a PC4 with router and sub-client. Confirm sub-client on PC4 receives data from pub-peer on PC1.

## Phase 6 - Demo application 2 - Rover

- Mars Rover (joystick teleop, FPV, mission control)

## Phase 7 - Utilities

- utility: hostname, hostaddress, isportinuse, programname, programpath, execute, flag_set
- file cache
- md5sum
- factory using crtp (see scratch)
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
