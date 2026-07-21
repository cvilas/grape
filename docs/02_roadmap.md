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
- [ ] Support zero-copy in IPC
- [ ] [Remote teleop](../modules/experimental/robot/locomotion/teleop/README.md)
- [ ] Audio support for [camera](../modules/camera/README.md)
- [ ] Basic OpenGL scenegraph
  - [ ] Clean up experimental implementation in scratch/scenegraph/copilot
  - [ ] Build scenegraph using data-oriented design
  - [ ] Extend to support asset loading using assimp
  - [ ] Design a text-based scenegraph description format using our scripting engine
  - [ ] Interactive point cloud viewer
- [ ] [Data recording](../modules/experimental/drake/README.md)
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
- [ ] CANOpen
- [ ] [CI jobs](../.github/workflows/README.md)
- [ ] Optimise the [ring buffers](https://rigtorp.se/ringbuffer/)


## References

- [MuJoCo tutorials](https://pab47.github.io/mujoco.html)
- [3D graphics programming](https://pikuma.com/courses/learn-3d-computer-graphics-programming)
