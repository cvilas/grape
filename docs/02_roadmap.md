# Roadmap

Goal: A catalog of useful tools and applications using X86-64 and Raspberry Pi as hardware platforms

## Completed

- Versioning
- Exception definitions
- Command line flags parsing
- Logging library
- Scripting library
- Message passing
- Realtime services
- Serialization
- Application framework

## Ongoing

- Implement queryable/query API
  - :done: Test the new tcp_pubsub API works
  - Write eCAL server example
    - refer to minimal_client.cpp and minimal_server.cpp
  - Implement our API
    - make it usable with zenoh queryable
  - Transform service example to use our API
- Implement [grapecam](https://github.com/cvilas/grapecam)
- Implement systemd service target
- Implement zero-copy read and write
- Implement Teleop controller
- Implement behaviour tree 

## Device IO

- HW IO
  - CANopen
  - joystick
  - midi
- Configure Raspberry Pi5 for realtime applications
  - Reference: [Low latency](https://ubuntu.com/blog/real-time-kernel-tuning).
  - Document how to allocate specified CPU cores to run Linux.
  - Document how to allocate specified CPU cores to run my processes and threads.
  - Demo application to show low latency and low jitter loop execution
- Study
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
  - [cactus-rt](https://github.com/cactusdynamics/cactus-rt/) on ROS2 interop
  - [glaze](https://github.com/stephenberry/glaze) for JSON serialisation and reflection
- Shared memory
- Single-producer multi-consumer queue using externally specified memory (heap or shared memory)
- Refactor multi-producer single-consumer queue
  - Compare `MPSCQueue` against `FIFOBuffer`. Remove one. 
  - Rename it for clarity
- Refactor thread class out of realtime and put it in 'grape'
  - Insert logging to capture timer overruns in the loop
- `reinterpret_cast<uintptr_t>` from `const T*` and then modifying it later is undefined behaviour. Fix `probe::PinConfig::pin`. Consider `std::start_lifetime_as` instead.
- replace `grape::realtime::SystemError` with `std::errc`

## Math library

- Delay line
- Low pass filter
- Differentiator
- Integrator
- Matrix operations
- Quaternion operations

## Visualisation

Goal: Scenegraph visualisation and plotting tools from first principles

- Choose Windowing API. Options:
  - SDL3: Better pipeline model that matches how GPUs work. Better support for GPU computing
  - GLFW: Simple
  - Qt6: Complete, including graphs
  - PoC: Implement a HW accelerated shaded triangle in all of them. Choose based on simplicity (verbosity, expressiveness), maintainability 
- plot
  - Generic plotting API and `plottable`concept
  - Implement PoC (reference: [Qt6 graphs](https://doc.qt.io/qt-6/qtgraphs-index.html))
- Study
  - [2D Game Engine](https://pikuma.com/courses/cpp-2d-game-engine-development)
  - [Vulkan in 30 minutes](https://renderdoc.org/vulkan-in-30-minutes.html)
  - [Vulkan Tutorial](https://vulkan-tutorial.com/)
  - [Vulkan guide](https://vkguide.dev/)
  - [Scenegraphs](https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph)
  - [Flecs and ECS](https://github.com/SanderMertens/flecs)
  - [CLoudPeek](https://github.com/Geekgineer/CloudPeek/tree/main): Could serve as a starting point for custom viewer
  - [USD](https://developer.nvidia.com/usd#nvidia)
  - glTF with [physics extensions](https://github.com/eoineoineoin/glTF_Physics)
- HW accelerated 3D graphics
  - Design scene description format using our scripting engine
    - Study [Anki](https://github.com/godlikepanos/anki-3d-engine) which uses Lua for scnegraph
  - Design scenegraph library using existing khronos libs
    - Review [type-erasure](https://github.com/cvilas/scratch/blob/master/type_erasure.cpp) as an abstraction technique for drawing shapes
    - Implement PoC. (Example: [Qt3d](https://github.com/cvilas/scratch/3dvis/qt))
    - Implement a basic scenegraph example and check performance in MacOS and Linux
- Disk recording and playback for time-series multi-modal data ([README](../modules/common/recorder/README.md))
  - [reference](https://github.com/basis-robotics/basis/tree/main/cpp/recorder)

## CI and build robustness

- Introduce [RTSan](https://clang.llvm.org/docs/RealtimeSanitizer.html)
- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions
- Implement CI build using github workflow  
- Integrate cpack to generate artifacts
- Support GCC builds without restrictions
- Support installing targets as systemd services


## Raspberry Pi Demo Applications

- AHRS
  - Use IMU from [Sensing hat](https://www.raspberrypi.com/products/sense-hat/)
- Environment monitor (CO2, temperature, humidity, pressure, light)
  - Use [Sensing hat](https://www.raspberrypi.com/products/sense-hat/)
- PoE camera
  - Use [HD camera](https://www.raspberrypi.com/products/raspberry-pi-global-shutter-camera/)
- LQR sim
  - Improvements to `probe::Monitor` (See TODO in [README](../modules/probe/monitor/README.md))
  - [MuJoCoPy Bootcamp](https://pab47.github.io/mujocopy.html) LQR sim from lesson 13, demonstrating integration of MujoCo, plotting and control
- Rover
  - DiY kit: [Rover](https://github.com/nasa-jpl/open-source-rover)
  - Demonstrate joystick teleop, FPV and mission control

## Zenoh

- Implement queryable/query API
- Implement zero-copy read and write
- Implement Reliable/BestEffort QoS
- Implement History QoS
- Define topics for matched example programs in a single place
- Implement PutOptions and subscriber Sample fields
  - Support attachments
  - Resolve how we can combine congestion control, priority and reliability settings in a coherent way to offer fewer choices at the user API layer?
    - See [discord](https://discord.com/channels/914168414178779197/940584045287460885/1311629493445853206)
  - Consider supporting sample kind (put/delete)
- Understand the point of on_drop callback in subscriber and support it if necessary
- Documentation cleanup: examples
- Understand hybrid logical clocks
- Support hybrid logical clocks implementation
- Fix zenoh examples: pull, shm pub/sub
- New zenoh examples: Router interceptors (downsampling), authentication, access control, serdes (ZBytes)
- PoC IPC experiments
  - Case 1: pub-peer on PC1, sub-peer on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 2: pub-peer + router on PC1, sub-peer + router on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 3: Extend case2 by adding a PC4 with router and sub-client. Confirm sub-client on PC4 receives data from pub-peer on PC1.

## References

- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- C++23 features: [cppcon](https://youtu.be/Cttb8vMuq-Y), [cpp weekly](https://youtu.be/N2HG___9QFI)
- C++20 features: <https://youtu.be/N1gOSgZy7h4>
- Practical [C++26 Reflection](https://youtu.be/cqQ7v6xdZRw)
- Interfaces with C++20 concept: <https://concepts.godbolt.org/z/PjGb466cr>
- Clean code: <https://youtu.be/9ch7tZN4jeI>
