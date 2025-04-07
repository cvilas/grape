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

## Current tasks

- IPC: Impmenent Python bindings [#131]
- IPC: Implement means to isolate IPC to a set of hosts [#129]
- IPC: Implement Query API [#130]
- IPC: Implement zero-copy read and write [#132]

## Camera

- Implement a PoC gstreamer capture and HW accelerated glfw display
- Implement [grapecam](https://github.com/cvilas/grapecam)

## CAN

- Implement canopen interface

## Teleop

- Implement Teleop controller
- Implement joystick interface

## Low latency applications on raspberry pi

- Document how to configure Raspberry Pi5 for realtime applications
- Document how to allocate specified CPU cores to run Linux.
- Document how to allocate specified CPU cores to run my processes and threads.
- Demo application to show low latency and low jitter loop execution on Pi
- Reference: [Low latency Ubuntu](https://ubuntu.com/blog/real-time-kernel-tuning)

## Data structures for realtime control

- Implement shared memory interface
- Implement single producer multi-consumer queue using heap and shared memory
- Refactor multi-producer single-consumer queue
  - Compare `MPSCQueue` against `FIFOBuffer`. Remove one. 
  - Rename it for clarity
- Refactor thread class out of realtime and put it in 'grape'
  - Insert logging to capture timer overruns in the loop
- `reinterpret_cast<uintptr_t>` from `const T*` and then modifying it later is undefined behaviour. Fix `probe::PinConfig::pin`. Consider `std::start_lifetime_as` instead.
- replace `grape::realtime::SystemError` with `std::errc`
- Implement behaviour tree 

## Math library for realtime control

- Delay line
- Low pass filter
- Differentiator
- Integrator
- Matrix operations
- Quaternion operations

## Robot model/kinematics/Visualisation

Goal: Robot model description file as the single source of truth to generate forward/inverse kinematics and scenegraph visualisation

- Choose Windowing API. Options:
  - SDL3: Better pipeline model that matches how GPUs work. Better support for GPU computing
  - GLFW: Simple
  - PoC: Implement a HW accelerated shaded triangle in both of them. Choose based on simplicity (verbosity, expressiveness), maintainability 
- Study
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
  - Auto generated robot kinematics from model
    - [Talk](https://youtu.be/CwN0I8yUqok?feature=shared)
    - [Code](https://github.com/pac48/fast_robot_kinematics)
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

## Recording and playback for time-series multi-modal data 

- [README](../modules/common/recorder/README.md)
- [Reference](https://github.com/basis-robotics/basis/tree/main/cpp/recorder)

## CI and build robustness

- Introduce [RTSan](https://clang.llvm.org/docs/RealtimeSanitizer.html)
- Setup configuration presets for developer and CI builds
  - Incorporate lessons from https://youtu.be/UI_QayAb9U0
  - Fail the CI if clang-format changes code
  - Add configuration presets to CMakePresets.json
  - Develop github CI build file
  - Document the usage in install instructions
- Implement CI build using github workflow  
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

## References

- [cactus-rt](https://github.com/cactusdynamics/cactus-rt/)
- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- [glaze](https://github.com/stephenberry/glaze) for JSON serialisation and reflection
- C++23 features: [cppcon](https://youtu.be/Cttb8vMuq-Y), [cpp weekly](https://youtu.be/N2HG___9QFI)
- C++20 features: <https://youtu.be/N1gOSgZy7h4>
- Practical [C++26 Reflection](https://youtu.be/cqQ7v6xdZRw)
- Interfaces with C++20 concept: <https://concepts.godbolt.org/z/PjGb466cr>
- Clean code: <https://youtu.be/9ch7tZN4jeI>
