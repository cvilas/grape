# Roadmap

## Manipulator

- [ ] Build [LeRobot SO-ARM101 kit](https://github.com/TheRobotStudio/SO-ARM100)
- [ ] Set up [LeRobot](https://huggingface.co/docs/lerobot/index)
- [ ] Learn to pick and place

## Walker

- [ ] Build [XGO-Mini kit](https://shop.elecfreaks.com/products/elecfreaks-cm4-xgo-mini-robot-dog-kit-for-raspberry-pi)
- [x] Implement joystick interface for Linux
- [x] Implement transactional teleop interface.  
- [x] Implement SDL3-based camera streamer
- [x] [picam](../modules/rpi/picam/README.md)  
- [x] Implement system clock
- [ ] RL walk in MuJoCo
- [ ] RL walk in reality

## Rover

- [ ] Build [DIY kit](https://github.com/nasa-jpl/open-source-rover)
- [ ] Implement Localiser (GPS + AHRS)
- [ ] Integrate long-distance comm link
- [ ] Implement telemetry: position, attitude, battery, camera
- [ ] Implement FPV control
- [ ] Implement dashboard view 

## QRTS

Create a qrts subtree, recreate QRTS libraries, relive 2001

- [ ] [qmotor](../modules/probe/concept/README.md)
- [ ] rp

## Libraries

- [ ] [Realtime control loop monitoring](../modules/probe/monitor/README.md)
- [ ] [drake](../modules/experimental/drake/README.md)
- [ ] [plot](../modules/experimental/plot/README.md)
- [ ] Implement 3D scenegraph libraries using SDL, OpenGL, Khronos libs
  - [ ] Review `Getting Started` section of OpenGL [book](https://learnopengl.com/)
  - [ ] Review my experimental implementations in scratch/scenegraph/copilot
  - [ ] Build version 1: Traditional scenegraph using class hierarchy
  - [ ] Study [Flecs and ECS](https://github.com/SanderMertens/flecs)
  - [ ] Build version 2: Modern scenegraph using ECS approach
  - [ ] Extend to support asset loading using assimp
  - [ ] Study [Anki](https://github.com/godlikepanos/anki-3d-engine) which uses Lua for scenegraph
  - [ ] Design a text-based scenegraph description format using our scripting engine
- [ ] [linalg](../modules/experimental/linalg/README.md)
- [ ] DSP functions
  - [ ] Implement signal processor [concept](https://concepts.godbolt.org/z/PjGb466cr)
  - [ ] Delay line
  - [ ] Butterworth LPF
  - [ ] Exponential LPF
  - [ ] Differentiator
  - [ ] Integrator
  - [ ] Velocity observer
- [ ] [ahrs](../modules/experimental/ahrs/README.md)
- [ ] [radar](../modules/experimental/radar/README.md)
- [ ] [halow](../modules/experimental/halow/README.md) 
- [ ] CANOpen
- [ ] Host monitoring micro-service
  - [ ] Implement library to read CPU, memory, disk usage, network utilisation, temperatures
  - [ ] Add systemd services support in GBS
  - [ ] Implement the ability to install monitoring micro-service using cpack
- [ ] CI updates (See TODO in [README](../.github/workflows/README.md))
- [ ] Support C++26
  - [ ] Auto serdes using [variadic structured bindings](https://youtu.be/qIDFyhtUMnQ)
  - [ ] Cross language [binding](https://godbolt.org/z/bYPcjMd9q) to functions for scripting
  - [ ] Automatic differentiation using reflection
  - [ ] Message dispatch using [pattern matching](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1371r3.pdf)
  - [ ] _this_ parameter in member functions for [dependency injection](https://www.linkedin.com/pulse/c26s-game-changing-features-memory-constrained-systems-lourette-xqd5e/)
  - [ ] Static reflection in [embedded messaging protocols](https://www.linkedin.com/pulse/eliminating-dynamic-memory-embedded-protocols-c26-static-lourette-sio1e/)
  - [ ] Study [Catch23](https://github.com/philsquared/Catch23) when it is still simple, and replace Catch2
- [ ] IPC: Fix ipc in MacOS
  - [ ] `grape_ipc_perf_pub`/`sub`
  - [ ] `grape_camera_pub`/`sub`
- [ ] Optimise the [ring buffer](https://rigtorp.se/ringbuffer/)

## Study

- Performance
  - Concurrency (many tasks at once) and parallelism (task broken into multiple pieces at once) 
    using senders and receivers ([std::execution](https://en.cppreference.com/w/cpp/execution)). 
  - Polymorphic resource allocators (see `std::pmr` namespace) and how to use them in embedded systems
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
- Robot model/kinematics/Visualisation.
  - High performance IK using [IK-Geo](https://alexanderelias.com/ur5-ik/)
  - Auto generated robot kinematics from model (single model format for kinematics and visualisation)
    - [Talk](https://youtu.be/CwN0I8yUqok?feature=shared)
    - [Code](https://github.com/pac48/fast_robot_kinematics)
- Transition to Vulkan from OpenGL
  - [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
  - [Vulkan 3D game engine from scratch](https://youtu.be/hSL9dCjwoCU)
  - [Vulkan in 30 minutes](https://renderdoc.org/vulkan-in-30-minutes.html)
  - [Vulkan Tutorial](https://vulkan-tutorial.com/)
  - [Vulkan guide](https://vkguide.dev/)
  - Roadmap to reimplement scenegraph in Vulkan

## References

- MIT 6.4210 Robotic Manipulation [[course notes](https://manipulation.csail.mit.edu/)][[video lectures](https://www.youtube.com/watch?v=PGY-4LOPs7U)]
- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- Practical [C++26 Reflection](https://youtu.be/cqQ7v6xdZRw)
- [VULKAN: From 2D to 3D // C++ 3D Multiplayer Game From Scratch // LIVE TUTORIAL](https://youtu.be/hSL9dCjwoCU)
- [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
- [LLVm's realtime safety revolution](https://youtu.be/b_hd5FAv1dw)
