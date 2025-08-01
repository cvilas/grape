# Roadmap

Application driven development roadmap, with Raspberry Pi as the target hardware platform for deployment

## Towards a Rover

- [x] Implement joystick interface for Linux
- [x] Implement transactional teleop interface.  
- [ ] Create IPC endpoints templated on TopicAttributes concept
  - [ ] Rename current Publisher to RawPublisher. Likewise for subscriber
  - [ ] Create `Publisher<TopicAttributes>`. Likewise for subscriber.
  - [ ] Apply concept that TopicAttributes must contain data type and topic name specification
  - [ ] Eliminate the need to call ipc::init if defaults are desired (see syslog::init)
- [ ] Implement SDL3-based camera streamer (See [README](../modules/camera/README.md))
- [ ] Implement 3D scenegraph libraries using SDL, OpenGL, Khronos libs
  - [ ] Study [scenegraphs](https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph)
  - [ ] Review [type-erasure](https://github.com/cvilas/scratch/blob/master/type_erasure.cpp) as an abstraction technique for drawing shapes
  - [ ] Review my experimental implementations in scratch/scenegraph/copilot
  - [ ] Build version 1: Traditional scenegraph using class hierarchy
  - [ ] Build version 2: Modern scenegraph using ECS approach
  - [ ] Extend to support asset loading using assimp
  - [ ] Study [Flecs and ECS](https://github.com/SanderMertens/flecs)
  - [ ] Study [USD](https://developer.nvidia.com/usd#nvidia)
  - [ ] Study [glTF with physics extensions](https://github.com/eoineoineoin/glTF_Physics)
  - [ ] Study [Anki](https://github.com/godlikepanos/anki-3d-engine) which uses Lua for scenegraph
  - [ ] Choose a model description format
  - [ ] Design a text-based scenegraph description format using our scripting engine
- [ ] Math library: 
  - [ ] constexpr matrix and quaternions operations to support AHRS implementation
  - [ ] linear algebra using std::mdspan and c++26 linalg 
- [ ] AHRS using [sense hat](https://www.raspberrypi.com/products/sense-hat/)
- [ ] DSP functions
  - [ ] Implement signal processor [concept](https://concepts.godbolt.org/z/PjGb466cr)
  - [ ] Delay line
  - [ ] Low pass filter
  - [ ] Differentiator
  - [ ] Integrator
  - [ ] Velocity observer
- [ ] [Prepare Pi for realtime control](../modules/common/realtime/README.md) 
- [ ] Real-time time-series plotting using implot (redesign from scratch again)
- [ ] [Realtime control loop monitoring](../modules/probe/monitor/README.md)
- [ ] Propose client-server messaging interface for robot on-board network 
- [ ] Robustify IPC across hosts (specifically TCP across hosts do not work well enough)
- [ ] Implement [picam](../modules/rpi/picam/README.md)  
- [ ] Host monitoring micro-service
  - [ ] Implement library to read CPU, memory, disk usage, network utilisation, temperatures
  - [ ] Add systemd services support in GBS
  - [ ] Implement the ability to install monitoring micro-service using cpack
  - [ ] IPC: Implement means to isolate IPC to a set of hosts [#129]
  - [ ] IPC: Implement zero-copy read and write [#132]
  - [ ] IPC: Fix 'local' mode communication in MacOS
- [ ] Integrate [radar](https://shop.pimoroni.com/products/dream-hat-plus-radar?variant=55529907290491)
- [ ] Rover
  - [ ] Build [DIY kit](https://github.com/nasa-jpl/open-source-rover)
  - [ ] Implement CANOpen interface for Linux
  - [ ] Steam Deck as operator controller: joystick teleop, FPV and mission control
  - [ ] Record and playback time-series multi-modal data
- [ ] CI updates (See TODO in [README](../.github/workflows/README.md))
- [ ] Support C++26
  - [ ] Auto serdes using [variadic structured bindings](https://youtu.be/qIDFyhtUMnQ)
  - [ ] Cross language [binding](https://godbolt.org/z/bYPcjMd9q) to functions for scripting
  - [ ] Message dispatch using [pattern matching](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1371r3.pdf)
  - [ ] _this_ parameter in member functions for [dependency injection](https://www.linkedin.com/pulse/c26s-game-changing-features-memory-constrained-systems-lourette-xqd5e/)

## Study

- Performance
  - [std::execution](https://en.cppreference.com/w/cpp/execution). Play with reference implementation [stdexec](https://github.com/NVIDIA/stdexec)
  - Polymorphic resource allocators (see `std::pmr` namespace) and how to use them in embedded systems
  - [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)
- Robot model/kinematics/Visualisation. (Goal: Single model description format for kinematic calculations and scenegraph visualisation)
  - Auto generated robot kinematics from model
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
