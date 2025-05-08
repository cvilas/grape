# Roadmap

Goal: A catalog of useful tools and applications using X86-64 and Raspberry Pi as hardware platforms

- [x] Versioning
- [x] Exception definitions
- [x] Command line flags parsing
- [x] Logging library
- [x] Scripting library
- [x] Message passing
- [x] Serialization
- [x] Application framework

## Useful things first

- [ ] RaspberryPi camera pipeline (libcamera source, SDL sink). Use cases: distributed sensors, lab test recorder
- [ ] Host monitoring library (CPU, memory, network, battery, temperatures)
- [ ] Systemd services support. Use case: Host monitoring micro-service
- [ ] Transactional teleop interface (packet based to switch modes, command velocity, flip flags).  
- [ ] RaspberryPi sense-hat based AHRS and input device. Use case: Teach pendant for imitation learning
- [ ] imgui as a separate static-only library. Use case: Simple UI for monitoring signals
- [ ] Joystick interface in Linux
- [ ] CANOpen interface for Linux
- [ ] Math library
- [ ] DSP functions (integrator, velocity observer, differentiators, filters)

## Camera

- [ ] view
- [ ] stream (capture -> pub/sub -> view)
- [ ] record

## IPC 

- [ ] IPC: Implement means to isolate IPC to a set of hosts [#129]
- [ ] IPC: Implement Service/Query API [#130]
- [ ] IPC: Implement zero-copy read and write [#132]
- [ ] IPC: Fix 'local' mode communication in MacOS
- [ ] Record and playback time-series multi-modal data

## Low latency applications on raspberry pi

- [ ] Document how to configure Raspberry Pi5 for realtime applications
- [ ] Document how to allocate specified CPU cores to run Linux.
- [ ] Document how to allocate specified CPU cores to run my processes and threads.
- [ ] Demo application to show low latency and low jitter loop execution on Pi
- Reference: [Low latency Ubuntu](https://ubuntu.com/blog/real-time-kernel-tuning)

## Robot model/kinematics/Visualisation

Goal: Robot model description file format as the single source of truth for kinematic calculations and scenegraph visualisation

- Study
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
  - Auto generated robot kinematics from model
    - [Talk](https://youtu.be/CwN0I8yUqok?feature=shared)
    - [Code](https://github.com/pac48/fast_robot_kinematics)
  - [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
  - [Vulkan 3D game engine from scratch](https://youtu.be/hSL9dCjwoCU)
  - [Vulkan in 30 minutes](https://renderdoc.org/vulkan-in-30-minutes.html)
  - [Vulkan Tutorial](https://vulkan-tutorial.com/)
  - [Vulkan guide](https://vkguide.dev/)
  - [Scenegraphs](https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph)
  - [Flecs and ECS](https://github.com/SanderMertens/flecs)
  - [USD](https://developer.nvidia.com/usd#nvidia)
  - glTF with [physics extensions](https://github.com/eoineoineoin/glTF_Physics)
- HW accelerated 3D graphics
  - Design scene description format using our scripting engine
    - Study [Anki](https://github.com/godlikepanos/anki-3d-engine) which uses Lua for scnegraph
  - Design scenegraph library using existing khronos libs
    - Review [type-erasure](https://github.com/cvilas/scratch/blob/master/type_erasure.cpp) as an abstraction technique for drawing shapes
    - Implement PoC. (Example: [Qt3d](https://github.com/cvilas/scratch/3dvis/qt))
    - Implement a basic scenegraph example and check performance in MacOS and Linux

## Data structures for realtime control

- Study: 
  - [std::execution](https://en.cppreference.com/w/cpp/execution) and play with reference implementation [stdexec](https://github.com/NVIDIA/stdexec)
  - Polymorphic resource allocators (see `std::pmr` namespace) and how to use them in embedded systems
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

## Math library

- [ ] Study: Interfaces with C++20 concept: <https://concepts.godbolt.org/z/PjGb466cr>
- [ ] Delay line
- [ ] Low pass filter
- [ ] Differentiator
- [ ] Integrator
- [ ] Matrix operations
- [ ] Quaternion operations

## CI and build robustness

- [ ] Introduce [RTSan](https://clang.llvm.org/docs/RealtimeSanitizer.html)
- [ ] Setup configuration presets for developer and CI builds
- [ ] Implement CI build using github workflow    
- [ ] Support installing targets as systemd services

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
  - Implement joystick interface
  - Implement canopen interface

## References

- [cactus-rt](https://github.com/cactusdynamics/cactus-rt/)
- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- [glaze](https://github.com/stephenberry/glaze) for JSON serialisation and reflection
- C++23 features: [cppcon](https://youtu.be/Cttb8vMuq-Y), [cpp weekly](https://youtu.be/N2HG___9QFI)
- C++20 features: <https://youtu.be/N1gOSgZy7h4>
- Practical [C++26 Reflection](https://youtu.be/cqQ7v6xdZRw)
- Clean code: <https://youtu.be/9ch7tZN4jeI>
- [How to start a modern C++ project - Mikhail Svetkin - Meeting C++ 2023](https://youtu.be/UI_QayAb9U0)
- [VULKAN: From 2D to 3D // C++ 3D Multiplayer Game From Scratch // LIVE TUTORIAL](https://youtu.be/hSL9dCjwoCU)
- [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
