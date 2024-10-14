# README: recorder

## Brief

Disk recording and playback facility

## Detailed description

### Requirements

- Interoperable across C++ and Python
- Independent of serialisation (i.e., API works on raw data streams)
- Supports arbitrary number of data sources updating at arbitrary data-rates up to 1kHz
- Able to record and replay at source data rates
- Able to attach arbitrary user-defined metadata to data streams. Use-cases include:
  - Text description
  - Schema
- API independent of but adaptable to pub/sub IPC

### TODO

- :done: Requirements analysis
- Research on what exists to record and replay timeseries multimodal data  
- Trial [mcap](https://mcap.dev/)
  - Develop CMakeLists.txt. See [mcap_vendor](https://github.com/ros2/rosbag2/blob/rolling/mcap_vendor/) and [mcap_builder](https://github.com/olympus-robotics/mcap_builder/)
  - write to disk
  - read from disk
  - benchmark
- Evaluate [rerun](https://rerun.io/)
- Review [DataTamer](https://github.com/PickNikRobotics/data_tamer). Development seems to be stalled on this project.
- IPC, serdes and record/replay go together in a robotic system. Develop PoC as follows
  - Plant produces sensor data on different IPC channels
  - Data is recorded to disk
  - Data is played back from disk
  - Data is transmitted back on IPC as if originating from sensor sources
  
