# Roadmap

## Libraries

- [x] [Console IO](../modules/common/conio/README.md)
- [x] [Scripting](../modules/common/script/README.md)
- [x] [Logging](../modules/common/log/README.md)
- [x] [Serialisation](../modules/common/serdes/README.md)
- [x] [IPC](../modules/common/ipc/README.md)
- [x] [Joystick](../modules/common/joystick/README.md)
- [x] [Camera](../modules/camera/README.md)
- [x] [Clock](../modules/common/clock/README.md)
- [x] [Picam](../modules/rpi/camera/README.md)  
- [x] [Pisense](../modules/rpi/sense_hat/README.md)
- [x] [Plot](../modules/common/plot/README.md)
- [ ] [AHRS](../modules/experimental/ahrs/README.md)
- [ ] [Linalg](../modules/experimental/linalg/README.md)
- [ ] Secure teleop (encryption and authentication)
- [ ] Audio support for [camera](../modules/camera/README.md)
- [ ] Basic OpenGL scenegraph
  - [ ] Clean up experimental implementation in scratch/scenegraph/copilot
  - [ ] Build scenegraph using data-oriented design
  - [ ] Extend to support asset loading using assimp
  - [ ] Design a text-based scenegraph description format using our scripting engine
- [ ] [Data recording](../modules/experimental/drake/README.md)
- [ ] Interactive point cloud viewer
- [ ] Advanced serialisation
  - [ ] Auto serdes using reflection and [variadic structured bindings](https://github.com/cvilas/scratch/blob/master/variadic_bindings.cpp)
  - [ ] Cross language [binding](https://godbolt.org/z/bYPcjMd9q) to functions for scripting
  - [ ] Plotting serialised topic data directly as `grape_plot --topics="/some/topic/name","/another/topic/name"`
- [ ] [Realtime control loop monitoring](../modules/probe/monitor/README.md)
- [ ] Integrate LLVM realtime sanitizer
- [ ] DSP functions
  - [ ] Implement signal processor [concept](https://concepts.godbolt.org/z/PjGb466cr)
  - [ ] Delay line
  - [ ] Butterworth LPF
  - [ ] Exponential LPF
  - [ ] Differentiator
  - [ ] Integrator
  - [ ] Velocity observer
- [ ] Multimodal telemetry: 
  - 1x audio channel, 2x 1080p30 video channel, 1x bidirectional data channel (teleop), 3x unidirectional data channel for structured & unstructured data
  - teleop control authentication and arbitration
- [ ] CANOpen
- [ ] CI updates (See TODO in [README](../.github/workflows/README.md))
- [ ] Optimise the [ring buffers](https://rigtorp.se/ringbuffer/)

## Study

- std::simd
- [std::execution](https://github.com/NVIDIA/stdexec)
- std::meta
- std::linalg
- std::hive
- std::debugging
- std::polymorphic & std::indirect
- std::is_within_lifetime
- RCU (read-copy-update)
- std::text_encoding
- std::strong_ordering
- new SI ratio prefixes
- parallel range algorithms
- pointer provenance
- Polymorphic resource allocators (`std::pmr` namespace) and how to use them in embedded systems
- [Robotics at compile time](https://youtu.be/Y6AUsB3RUhA)

## References

- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
