# Roadmap

- Base library
  - :done: Versioning
  - Custom Exception definitions
- Logging library: std::clog based, multiple sinks with std::streambuf
- console io library
  - Command line flags parsing
  - kbhit and getch
- Configuration library: `Configurable` concept using toml::table
  
  ```c++
  template<class T>
  concept Configurable = requires (T a) { a.configure(const toml::table&); }
  ```

- Realtime: POSIX scheduling wrappers
- timing: periodic timer, watchdog, stopwatch. loop timer
- utility: hostname, hostaddress, programname, programpath, typename, demangle, execute, magic_enum, flag_set
- IPC
  - serdes
  - 0MQ wrapper
- plot:
  - `plottable` concept
  - Uses ImGui (https://github.com/ocornut/imgui) and ImPlot (https://github.com/epezent/implot)
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

Use [C++20](https://youtu.be/N1gOSgZy7h4) or newer features in development

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
- std::stacktrace
