# Roadmap

## Phase 1 - Basics

- :done: Versioning
- :done: Exception definitions
- :done: Command line flags parsing
- :done: Logging library
- :done: Scripting library
- :done: Introduce IPC
- Command line parsing part 2:
  - addOption(name, optional|required, default);
  - support --help option by default
  - support program description as constructor parameter

## Phase 2 - Demo application 1 - Probe

- study: https://pikuma.com/courses/cpp-2d-game-engine-development
- serdes
  - Choose backend: low overhead, type-safe, fast, no external dependencies, supports C++ and Python
  - concept `serialisable`
- plot:
  - :done: Choose a backend: implot
  - concept `plottable`
- probe library:
  - Take inspiration from PlotJuggler and DataTamer
  - Port github.com/cvilas/probe library
  - *grape_plant*: closed loop control, deployable on embedded processors.
  - *grape_supervisor*: remote monitoring, graphing, online parameter tuning, and event logging; runs as a separate process, likely on a different host.

## Phase 3 - CI

- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions

## Phase 4 - Audio, Video and 3D Graphics

- Audio/Video streaming:
  - Choose backend for audio/video device handling and stream processing
  - Implement basic examples for AV capture, streaming and display

- HW accelerated 3D graphics
  - Select a backend: vsg, ogre, something else
  - Implement a basic scenegraph example and check performance in MacOS and Linux VM
  - Implement scenegraph in our scripting language and have it render by the backend

## Phase 5 - Robotics Core

- Timing: periodic timer, watchdog, stopwatch. loop timer
- enum-to-string and string-to-enum
  - enum to string: Copy the general idea from here: <https://godbolt.org/z/6MxYznfbf>
  - consider magic_enum and other options and choose one
  - refactor existing `toString()` functions
- Additional zenoh examples
  - zenoh_pub_cache: Understand what it is supposed to do and make it work
  - zenoh_query_sub - make it work
  - z_storage
  - zenoh video capture/display: https://github.com/eclipse-zenoh/zenoh-demos/tree/master/computer-vision/zcam
- HW IO
  - CANopen
  - joystick
- Behaviour trees: Consider building from first principles
- FSM: introspectable, visualise state transition graph using graphviz.

## Phase 6 - Demo application 2 - Rover

- Mars Rover (joystick teleop, FPV, mission control)

## Phase 7 - Utilities

- Realtime: POSIX scheduling wrappers
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
