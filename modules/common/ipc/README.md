# README: ipc

## Brief

Intra-process, inter-process and inter-host communication

## Design considerations

- Minimal API that supports following messaging patterns:
  - pub-sub
  - query (remote procedure/service call)
- Automatic discovery and connectivity between matched endpoints
- Configurable for low latency
- Supports high throughput (eg: HD video streams)
- Reliable: Messages get through and in the right order
- Supports non-blocking write operations
- Datatype agnostic: Just provides mechanism to transport raw bytes
- Performant: Minimises data copies
- Scalable to 1000s of endpoints on WiFi without hidden costs (meta data traffic)
- Provides mechanisms to track liveliness of endpoints
- Provides mechanisms to isolate different systems on the same host or network
- Supports interoperability across C++ and Python.

## Zenoh

[Zenoh](https://zenoh.io/docs/overview/what-is-zenoh/) was chosen as the initial candidate for IPC after considering a [few options](./docs/ipc_options.md). See [examples](./examples/README.md) for usage patterns and capabilities.

### Copyright

Use of Zenoh within this project satisfies the terms and conditions of Apache License version 2.0 under which it is distributed.

## TODO

- :done: Phase 1: Basic implementation of Session, Publisher, Subscriber
- Phase 2
  - Implement `IPAddress::fromString()`
  - Move `IPAddress` and associated functions to utils
  - Implement client pub and sub
  - Implement query pub and sub
  - Implement ping, pong
  - Implement throughput pub and sub
  - Implement explicit error handling (don't leave it to default)
  - Avoid copy in createDataCallback() by using SpliceIterator
  - Raise MR
- Phase 3
  - Implement liveliness
  - Implement PutOptions and subscriber Sample fields
    - Support attachments
    - Support timestamping
    - Support priority
    - Support congestion control
    - Support reliability
    - Support for sample kind (put/delete)
  - Understand the point of on_drop callback in subscriber and use it if necessary
  - Implement shared memory
  - Implement caching
  - Raise MR
- Phase 4
  - Documentation cleanup
  - Unit tests
  - Lua utilities: hostname
