# Roadmap

## Phase 1 - Basics
- :done: Versioning
- :done: Exception definitions
- :done: Command line flags parsing
- :done: Logging library
- :done: Scripting library
- integrate magic_enum
  - refactor existing `toString()` functions
- serdes
  - concept `serialisable`
- IPC
  - :done: Select a backend
  - Build wrapper for backend

## Phase 2 - Graphics Core
- plot:
  - concept `plottable`
  - Uses ImGui (https://github.com/ocornut/imgui) and ImPlot (https://github.com/epezent/implot)
- HW accelerated 3D graphics
  - Select a backend
  - Implement a basic scenegraph example and check performance in MacOS and Linux VM

## Phase 3 - Robotics Core

- Realtime: POSIX scheduling wrappers
- timing: periodic timer, watchdog, stopwatch. loop timer
- utility: hostname, hostaddress, isportinuse, programname, programpath, typename, demangle, execute, flag_set
  - enum to string: Copy the general idea from here: https://godbolt.org/z/6MxYznfbf
- probe library:
  - Port github.com/cvilas/probe library
  - *grape_plant*: closed loop control, deployable on embedded processors.
  - *grape_supervisor*: remote monitoring, graphing, online parameter tuning, and event logging; runs as a separate process, more likely it runs on a different host.
- HW IO
  - canopen
  - joystick
- file cache
- md5sum  
- factory using crtp (see scratch)
- FSM
- Behaviour trees
- Video streaming: gstreamer
- [ftxui](https://github.com/ArthurSonzogni/FTXUI) based terminal UI apps 

Use C++20 or newer features in development

- references:
  - C++20 features: https://youtu.be/N1gOSgZy7h4
  - Clean code: https://youtu.be/9ch7tZN4jeI?si=YkO84hmfQWfq8KO8
- Designated initialisers
- 3-way comparison (spaceship)
- Concepts
- **Coroutines**: Review usability for async processing, nonblocking IO
- Modules
- std::format
- Source location
- Calendar updates in chrono
- Ranges
- constexpr everywhere
- span
- jthread
- use std::exception for truly exceptional errors, use std::expected everywhere else for expected exceptions (eg: error opening a file, serdes error, etc) and compose using monads to chain operations
  ```c++
  std::expected<Movie,Error> readMovie(const std::string& fileName){
    return openFile(fileName)
      .and_then([&](std::ifstream& file){ return readLine(file); })
      .and_then([&](const std::string& line){ return parseMovie(line); });
  }
  ```
- Provide separate interface headers and implementation headers. (See `clean code` video above)
- IoC containers for dependency injection, especially for mocking in tests: https://github.com/ybainier/Hypodermic. For why we should use it, see `clean code` video above
- std::stacktrace
