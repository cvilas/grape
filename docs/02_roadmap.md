# Roadmap

## Manipulator

- [ ] Build [LeRobot SO-ARM101 kit](https://github.com/TheRobotStudio/SO-ARM100)
- [ ] Set up [LeRobot](https://huggingface.co/docs/lerobot/index)
- [ ] Learn to pick and place

## Walker

- [ ] Build [XGO-Mini kit](https://shop.elecfreaks.com/products/elecfreaks-cm4-xgo-mini-robot-dog-kit-for-raspberry-pi)
- [ ] RL walk in MuJoCo
- [ ] RL walk in reality

## Rover

- [ ] Build [DIY kit](https://github.com/nasa-jpl/open-source-rover)
- [ ] Implement Localiser (GPS + AHRS)
- [ ] Integrate long-distance comm link
- [ ] Implement telemetry: position, attitude, battery, camera
- [ ] Implement FPV control
- [ ] Implement dashboard view 

## Libraries

- [x] [Console IO](../modules/common/conio/README.md)
- [x] [Scripting](../modules/common/script/README.md)
- [x] [Logging](../modules/common/log/README.md)
- [x] [Serialisation](../modules/common/serdes/README.md)
- [x] [IPC](../modules/common/ipc/README.md)
- [x] [Joystick](../modules/common/joystick/README.md)
- [x] [Teleop](../modules/experimental/robot/locomotion/teleop/README.md)  
- [x] [Camera](../modules/camera/README.md)
- [x] [Picam](../modules/rpi/camera/README.md)  
- [x] [Clock](../modules/experimental/robot/ego_clock/README.md)
- [x] [Pisense](../modules/rpi/sense_hat/README.md)
- [ ] [AHRS](../modules/experimental/ahrs/README.md)
- [ ] Basic OpenGL scenegraph
  - [ ] Clean up experimental implementation in scratch/scenegraph/copilot
  - [ ] Build scenegraph using data-oriented design
  - [ ] Extend to support asset loading using assimp
  - [ ] Design a text-based scenegraph description format using our scripting engine
- [ ] Ring buffers
    - [ ] SPMC ring buffer
    - [ ] MPSC ring buffer
    - [ ] Optimise the [ring buffer](https://rigtorp.se/ringbuffer/)
- [ ] [Linalg](../modules/experimental/linalg/README.md)
- [ ] [Plot](../modules/experimental/plot/README.md)
- [ ] [Realtime control loop monitoring](../modules/probe/monitor/README.md)
- [ ] [Data recording](../modules/experimental/drake/README.md)
- [ ] DSP functions
  - [ ] Implement signal processor [concept](https://concepts.godbolt.org/z/PjGb466cr)
  - [ ] Delay line
  - [ ] Butterworth LPF
  - [ ] Exponential LPF
  - [ ] Differentiator
  - [ ] Integrator
  - [ ] Velocity observer
- [ ] CANOpen
- [ ] CI updates (See TODO in [README](../.github/workflows/README.md))
- [ ] Support C++26
  - [ ] Auto serdes using [variadic structured bindings](https://youtu.be/qIDFyhtUMnQ)
  - [ ] Plotting serialised topic data directly as `grape_plot --topics="/some/topic/name","/another/topic/name"`
  - [ ] Cross language [binding](https://godbolt.org/z/bYPcjMd9q) to functions for scripting
  - [ ] Automatic differentiation using reflection
  - [ ] Message dispatch using [pattern matching](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1371r3.pdf)
  - [ ] _this_ parameter in member functions for [dependency injection](https://www.linkedin.com/pulse/c26s-game-changing-features-memory-constrained-systems-lourette-xqd5e/)
  - [ ] Static reflection in [embedded messaging protocols](https://www.linkedin.com/pulse/eliminating-dynamic-memory-embedded-protocols-c26-static-lourette-sio1e/)
  - [ ] Study [Catch23](https://github.com/philsquared/Catch23) when it is still simple, and replace Catch2

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

- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- Practical [C++26 Reflection](https://youtu.be/cqQ7v6xdZRw)
- [VULKAN: From 2D to 3D // C++ 3D Multiplayer Game From Scratch // LIVE TUTORIAL](https://youtu.be/hSL9dCjwoCU)
- [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
- [LLVm's realtime safety revolution](https://youtu.be/b_hd5FAv1dw)
