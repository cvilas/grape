# README: recorder

## Brief

Disk recording and playback facility

## Detailed description

### Requirements

- Interoperable across C++ and Python
- Independent of serialisation (i.e., API works on raw data streams)
- Supports arbitray number of data sources updating at arbitrary data-rates up to 1kHz
- Able to record and replay at source data rates
- Able to attach arbitrary user-defined metadata to data streams. Use-cases include:
  - Text description
  - Schema
- API independent of but adaptable to pub/sub IPC

### TODO

- :done: Requirements analysis
- Narrow down options: [mcap](https://mcap.dev/), [zenoh storage](https://github.com/eclipse-zenoh/zenoh-backend-filesystem), others
- IPC, serdes and record/replay go together in a robotic system. Develop PoC as follows
  - Plant produces sensor data on different IPC channels
  - Data is recorded to disk
  - Data is played back from disk
  - Data is transmitted back on IPC as if originating from sensor sources
  