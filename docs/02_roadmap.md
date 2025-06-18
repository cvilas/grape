# Roadmap

Application driven development roadmap, with Raspberry Pi as the target hardware platform for deployment

## Towards a Rover

- [x] Implement joystick interface for Linux
- [ ] Implement transactional teleop interface (packet based to switch modes, command velocity, flip flags).  
- [ ] [Stand-alone PoE camera](../modules/rpi/picam/README.md)  
- [ ] [Prepare Pi for realtime control](../modules/common/realtime/README.md) 
- [ ] 3D scenegraph using SDL
- [ ] Real-time time-series plotting using implot
- [ ] Math library: constexpr matrix and quaternions operations to support AHRS implementation
- [ ] AHRS using [sense hat](https://www.raspberrypi.com/products/sense-hat/)
- [ ] DSP functions
  - [ ] Implement signal processor [concept](https://concepts.godbolt.org/z/PjGb466cr)
  - [ ] Delay line
  - [ ] Low pass filter
  - [ ] Differentiator
  - [ ] Integrator
  - [ ] Velocity observer
- [ ] [Realtime control loop monitoring](../modules/probe/monitor/README.md)
- [ ] Host monitoring micro-service
  - [ ] Implement library to read CPU, memory, disk usage, network utilisation, temperatures
  - [ ] Add systemd services support in GBS
  - [ ] Implement the ability to install monitoring micro-service using cpack
  - [ ] IPC: Implement means to isolate IPC to a set of hosts [#129]
  - [ ] IPC: Implement zero-copy read and write [#132]
  - [ ] IPC: Fix 'local' mode communication in MacOS
- [ ] Rover
  - [ ] Build [DIY kit](https://github.com/nasa-jpl/open-source-rover)
  - [ ] Implement CANOpen interface for Linux
  - [ ] Steam Deck as operator controller: joystick teleop, FPV and mission control
  - [ ] Record and playback time-series multi-modal data
- [ ] CI updates (See TODO in [README](../.github/workflows/README.md))
- [ ] Support C++26
  - [ ] Auto serdes using [variadic structured bindings](https://youtu.be/qIDFyhtUMnQ)

## Study

- Performance
  - [std::execution](https://en.cppreference.com/w/cpp/execution). Play with reference implementation [stdexec](https://github.com/NVIDIA/stdexec)
  - Polymorphic resource allocators (see `std::pmr` namespace) and how to use them in embedded systems
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
- Robot model/kinematics/Visualisation. (Goal: Single model description format for kinematic calculations and scenegraph visualisation)
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
  - Study glTF with [physics extensions](https://github.com/eoineoineoin/glTF_Physics)
  - Study [Anki](https://github.com/godlikepanos/anki-3d-engine) which uses Lua for scnegraph

## Scenegraph for robotics computation and visualisation

- Design scene description format using our scripting engine
- Design scenegraph library using existing khronos libs
  - Review [type-erasure](https://github.com/cvilas/scratch/blob/master/type_erasure.cpp) as an abstraction technique for drawing shapes
  - Implement PoC. (Example: [Qt3d](https://github.com/cvilas/scratch/3dvis/qt))
  - Implement a basic scenegraph example and check performance in MacOS and Linux
- Define geometric primitives (plane, ellipsoid, cone, cuboid)

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
