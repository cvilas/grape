# Roadmap

## Completed

- Versioning
- Exception definitions
- Command line flags parsing
- Logging library
- Scripting library
- Message passing
- Realtime services
- Serialization

## Dec 2024

- Cross compile for Arm64 from X86 
  - with gcc toolchain
  - with clang toolchain
- grapecam
  - Implement grape::app. ([README](../modules/common/app/README.md))
  - Implement [grapecam](https://github.com/cvilas/grapecam)
  - Implement `isatty()` to identify terminal and print colored logs (see `cpptrace::istty`)
- plot
  - Generic plotting API and `plottable`concept
  - Implement PoC with [Qt6 Graphs](https://doc.qt.io/qt-6/qtgraphs-index.html)
- Record and replay ([reference](https://github.com/basis-robotics/basis/tree/main/cpp/recorder))
- Define behaviour for `experimental` flag in `declare_module` and implement it
- Review [ecal](https://github.com/eclipse-ecal/ecal). Is it an alternative to zenoh?
- delete `/tmp/ctcache-*` and fix linter warnings
- update `clang-tidy-cache`. Does it fix linter caching problems?

## Jan 2025

- Ability to create and destroy publishers and subscribers at will (required for teleop mux)
- CANOpen support
- Teleop controller

## Generalise IPC API

- Implement queryable/query API
- Avoid copy in createDataCallback() by using SpliceIterator
- Implement liveliness API
- Implement shared memory API
- Implement caching API
- Define topics for matched example programs in a single place
- Consider implementing pub-sub match callbacks
- Implement PutOptions and subscriber Sample fields
  - Support attachments
  - Support timestamping
  - Resolve how we can combine congestion control, priority and reliability settings in a coherent way to offer fewer choices at the user API layer?
    - See [discord](https://discord.com/channels/914168414178779197/940584045287460885/1311629493445853206)
  - Consider supporting sample kind (put/delete)
- Understand the point of on_drop callback in subscriber and support it if necessary
- Documentation cleanup: examples
- Understand hybrid logical clocks
- Support hybrid logical clocks implementation
- Disk recording and playback for time-series multi-modal data ([README](../modules/common/recorder/README.md))
- Fix zenoh examples: pull, shm pub/sub
- New zenoh examples: Router interceptors (downsampling), authentication, access control, serdes (ZBytes)
  
## Robotics core

- HW IO
  - CANopen
  - joystick
  - midi
- Configure Raspberry Pi5 for [low latency](https://ubuntu.com/blog/real-time-kernel-tuning). Document it.
- Study
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
  - [cactus-rt](https://github.com/cactusdynamics/cactus-rt/) on ROS2 interop
- Shared memory
- Single producer multiple consumer queue using externally specified memory (heap or shared memory)
- Consider removing `MPSCQueue`. It's unused. Rename `FIFOBuffer` to `MPSCQueue`
- Math library
  - Delay line
  - Low pass filter
  - Differentiator
  - Integrator
  - Matrix operations
- Behaviour trees: Consider building from first principles
- FSM: introspectable, visualisable state transition graph using graphviz.
- Introduce [RTSan](https://clang.llvm.org/docs/RealtimeSanitizer.html)
- Refactor thread class out of realtime and put it in 'grape'
  - Insert logging to capture timer overruns in the loop
- `reinterpret_cast<uintptr_t>` from `const T*` and then modifying it later is undefined behaviour. Fix `probe::PinConfig::pin`. Consider `std::start_lifetime_as` instead.
- replace `grape::realtime::SystemError` with `std::errc`

## 3D visualisation

- Study
  - [2D Game Engine](https://pikuma.com/courses/cpp-2d-game-engine-development)
  - [Vulkan in 30 minutes](https://renderdoc.org/vulkan-in-30-minutes.html)
  - [Vulkan Tutorial](https://vulkan-tutorial.com/)
  - [Vulkan guide](https://vkguide.dev/)
  - [Flecs and ECS](https://github.com/SanderMertens/flecs)
  - [CLoudPeek](https://github.com/Geekgineer/CloudPeek/tree/main): Could serve as a starting point for custom viewer
  - [USD](https://developer.nvidia.com/usd#nvidia)
  - glTF with [physics extensions](https://github.com/eoineoineoin/glTF_Physics)
- HW accelerated 3D graphics
  - Design scene description format using our scripting engine
    - Study [Anki](https://github.com/godlikepanos/anki-3d-engine) which uses Lua for scnegraph
  - Design scenegraph library using existing khronos libs
    - Implement PoC using Qt3D. See [scratch](https://github.com/cvilas/scratch)/3dvis/qt
    - Implement a basic scenegraph example and check performance in MacOS and Linux

## CI and build robustness

- Fix GCC builds
  - `std::expected` not available with -DENABLE_LINTER=ON
  - sccache breaks build with -DENABLE_CACHE=ON
- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions
- Implement CI build using github workflow  
- Integrate cpack to generate artifacts
- Integrate [ninjatracing](https://github.com/nico/ninjatracing)

## Demo applications

- Office environment (CO2, temperature, light) dashboard
- Network camera and viewer for industrial monitoring
- Zenoh interop with C++ publisher and Python subscriber, demonstrating data serialisation/deserialisation
- Improvements to `probe::Monitor` (See TODO in [README](../modules/probe/monitor/README.md))
- [MuJoCoPy Bootcamp](https://pab47.github.io/mujocopy.html) LQR sim from lesson 13, demonstrating integration of MujoCo, plotting and control
- [Rover](https://github.com/nasa-jpl/open-source-rover) demonstrating joystick teleop, FPV and mission control
- Implement advanced streaming
  - Choose backend for audio/video device handling and stream processing
  - Implement AV streaming server and client

## References

- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- C++23 features: [cppcon](https://youtu.be/Cttb8vMuq-Y), [cpp weekly](https://youtu.be/N2HG___9QFI)
- C++20 features: <https://youtu.be/N1gOSgZy7h4>
- Practical [C++26 Reflection](https://youtu.be/cqQ7v6xdZRw)
- Interfaces with C++20 concept: <https://concepts.godbolt.org/z/PjGb466cr>
- Clean code: <https://youtu.be/9ch7tZN4jeI>
