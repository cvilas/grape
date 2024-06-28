# Roadmap

## Phase 1 - Base

- :done: Versioning
- :done: Exception definitions
- :done: Command line flags parsing
- :done: Logging library
- :done: Scripting library
- :done: Message passing
- :done: Realtime services  

## Phase 2 - Multimodal data logging and visualisation - timeseries data

- :done: Implement `probe::Controller`
- :done: Implement `probe::Monitor` PoC
- Production-ready implementation of Monitor (See TODO in Monitor/README)

## Phase 3 - Multimodel data logging and visualisation - audio, video, 3D graphics

- Audio/Video streaming:
  - Choose backend for audio/video device handling and stream processing
  - Implement basic examples for AV capture, streaming and display
- zenoh video capture/display: https://github.com/eclipse-zenoh/zenoh-demos/tree/master/computer-vision/zcam
- HW accelerated 3D graphics
  - Select scene description format: [USD](https://developer.nvidia.com/usd#nvidia), glTF with [physics extensions](https://github.com/eoineoineoin/glTF_Physics)
  - Select a rendering backend: vsg, ogre, raylib, something else
  - Implement a basic scenegraph example and check performance in MacOS and Linux VM
  - Implement scenegraph in our scripting language and have it render by the backend
- Set zenoh version to 1.0.0
- Implement new zenoh features as examples
  - matched pub/sub discovery
  - Other new features: https://zenoh.io/blog/2024-04-30-zenoh-electrode/
- Demonstrate hard realtime capability with Ubuntu 24.04 on X86 and RaspberryPi
  - https://ubuntu.com/blog/real-time-kernel-tuning

## Phase 4 - Refactor

- Have a single place to maintain version numbers of all external dependencies
  - Single versions file maybe
  - Add version to find_package() calls within modules 
- Allow examples and tests to depend on external dependencies that the main project does not depend on
- Refactor gbs template files to make it project agnostic
  - Replace occurances of 'grape' with @CMAKE_PROJECT_NAME@ in all files
- Refactor logging
  - Color console handler like in [Quill](https://github.com/odygrd/quill/blob/master/quill/src/handlers/ConsoleHandler.cpp)
  - Study Quill to understand how to reduce overhead even more
- Refactor thread class out of realtime and put it in 'grape'
  - Insert logging to capture timer overruns in the loop
- Study [SPSC FIFO](https://youtu.be/K3P_Lmq6pw0) and review [implementation](https://github.com/CharlesFrasch/cppcon2023)
- Benchmark `FIFOBuffer` against [SPSC fifo](https://github.com/CharlesFrasch/cppcon2023) and improve performance where possible
- `reinterpret_cast<uintptr_t>` from `const T*` and then modifying it later is undefined behaviour. Fix `probe::PinConfig::pin`. Consider `std::start_lifetime_as` instead.

## Phase 5 - CI

- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions
- study
  - [boost-ext/reflect](https://github.com/boost-ext/reflect)
  - [reflect-cpp](https://github.com/getml/reflect-cpp)
- Implement CI build using github workflow  
- Integrate cpack to generate artifacts 
- Integrate [ninjatracing](https://github.com/nico/ninjatracing)
- Review all negated checks in `.clang-tidy`

## Phase 6 - Robotics behaviours

- Study
  - Robotics at compile time: https://youtu.be/Y6AUsB3RUhA
- enum-to-string and string-to-enum
  - enum to string: Copy the general idea from here: <https://godbolt.org/z/6MxYznfbf>
  - consider magic_enum and other options and choose one
  - refactor existing `toString()` functions
- HW IO
  - CANopen
  - joystick
- Behaviour trees: Consider building from first principles
- FSM: introspectable, visualise state transition graph using graphviz.
- PoC IPC experiments
  - Case 1: pub-peer on PC1, sub-peer on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 2: pub-peer + router on PC1, sub-peer + router on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 3: Extend case2 by adding a PC4 with router and sub-client. Confirm sub-client on PC4 receives data from pub-peer on PC1.

## Phase 7 - Demo application - Rover

- Mars Rover (joystick teleop, FPV, mission control)

## Phase 8 - Utilities

- serdes
  - Choose backend: low overhead, type-safe, fast, no external dependencies, supports C++ and Python
  - concept `serialisable`
- utility: hostname, hostaddress, isportinuse, programname, programpath, execute, flag_set
- file cache
- md5sum
- factory using crtp (see scratch)
- [ftxui](https://github.com/ArthurSonzogni/FTXUI) based terminal UI apps
- Consider integrating mp_uints library

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

## References

- C++23 features: [cppcon](https://youtu.be/Cttb8vMuq-Y), [cpp weekly](https://youtu.be/N2HG___9QFI)
- C++20 features: <https://youtu.be/N1gOSgZy7h4>
- Interfaces with C++20 concept: <https://concepts.godbolt.org/z/PjGb466cr>
- Clean code: <https://youtu.be/9ch7tZN4jeI>
- IoC containers for dependency injection, especially for mocking in tests: <https://github.com/ybainier/Hypodermic>. For why we should use it, see `clean code` video above
