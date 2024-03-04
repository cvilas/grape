# Roadmap

## Phase 1 - Base

- :done: Versioning
- :done: Exception definitions
- :done: Command line flags parsing
- :done: Logging library
- :done: Scripting library
- :done: Message passing
- :done: Realtime services

## Phase 2 - Multimodal data logging and visualisation - part 1

- :done: Implement `probe::Controller`
- :done: Implement `probe::Monitor` PoC

## Phase 3 - Multimodel data logging and visualisation - part 2

- Production-ready implementation of Monitor (See TODO in Monitor/README)
- [mcap](https://mcap.dev/) reader/writer
- Pin zenoh version to 1.0.0
  - Review zenohc and zenohcxx installation steps are correct in external/CMakeLists.txt.
  - Make sure static libs are built and installed when requested.
  - Implement new zenoh features as examples
- C++/Python interop
  - Choose serdes backend: low overhead, type-safe, fast
  - concept `serialisable`

## Phase 4 - Robotics core

- Configure Raspberry Pi5 [low latency](https://ubuntu.com/blog/real-time-kernel-tuning). Document it.
- Study
  - [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
  - [boost-ext/reflect](https://github.com/boost-ext/reflect)
  - [reflect-cpp](https://github.com/getml/reflect-cpp)
- enum-to-string and string-to-enum
  - enum to string: Copy the general idea from here: <https://godbolt.org/z/6MxYznfbf>
  - consider magic_enum and other options and choose one
  - refactor existing `toString()` functions
- Shared memory
- Single producer multiple consumer queue using externally specified memory (heap or shared memory)
- HW IO
  - CANopen
  - joystick
- Behaviour trees: Consider building from first principles
- FSM: introspectable, visualise state transition graph using graphviz.
- PoC IPC experiments
  - Case 1: pub-peer on PC1, sub-peer on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 2: pub-peer + router on PC1, sub-peer + router on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 3: Extend case2 by adding a PC4 with router and sub-client. Confirm sub-client on PC4 receives data from pub-peer on PC1.

## Phase 5 - Audio and Video streaming

- PoC zenoh video capture/display: https://github.com/eclipse-zenoh/zenoh-demos/tree/master/computer-vision/zcam
- Choose backend for audio/video device handling and stream processing
- Implement basic examples for AV capture, streaming and display

## Phase 6 - 3D graphics

- Study
  - [2D Game Engine](https://pikuma.com/courses/cpp-2d-game-engine-development)
  - [Vulkan in 30 minutes](https://renderdoc.org/vulkan-in-30-minutes.html)
  - [Vulkan Tutorial](https://vulkan-tutorial.com/)
  - [Vulkan guide](https://vkguide.dev/)
- HW accelerated 3D graphics
  - Select scene description format: [USD](https://developer.nvidia.com/usd#nvidia), glTF with [physics extensions](https://github.com/eoineoineoin/glTF_Physics)
  - Select a scenegraph engine: vsg, ogre, raylib, something else
  - Implement a basic scenegraph example and check performance in MacOS and Linux VM
  - Implement scenegraph in our scripting language and have it render by the backend

## Phase 7 - Refactor

- Support external dependencies on examples and tests that the main project does not depend on
- Refactor gbs template files to make it project agnostic
  - Replace occurances of 'grape' with @CMAKE_PROJECT_NAME@ in all files
- Refactor logging
  - Study [Quill](https://github.com/odygrd/quill) to understand how to reduce overhead even more
- Refactor thread class out of realtime and put it in 'grape'
  - Insert logging to capture timer overruns in the loop
- `reinterpret_cast<uintptr_t>` from `const T*` and then modifying it later is undefined behaviour. Fix `probe::PinConfig::pin`. Consider `std::start_lifetime_as` instead.

## Phase 8 - CI

- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions
- Implement CI build using github workflow  
- Integrate cpack to generate artifacts 
- Integrate [ninjatracing](https://github.com/nico/ninjatracing)
- Review all negated checks in `.clang-tidy`

## Phase 9 - Demo application - Rover

- Mars Rover (joystick teleop, FPV, mission control)

## Phase 10 - Utilities

- utility: hostname, hostaddress, isportinuse, programname, programpath, execute, flag_set
- file cache
- md5sum
- factory using crtp (see scratch)
- [ftxui](https://github.com/ArthurSonzogni/FTXUI) based terminal UI apps
- Consider integrating mp_units library

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
