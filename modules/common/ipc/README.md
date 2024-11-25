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

## TODO: Refactor to generalise the API

- :done: Phase 1: Basic implementation of Session, Publisher, Subscriber
- :done: Phase 2: router, client, throughput and latency examples
- Phase 3
  - Implement query pub and sub
  - Avoid copy in createDataCallback() by using SpliceIterator
  - Implement liveliness
  - Implement shared memory
  - Implement caching
  - Define topics for matched examples in a single place
  - Raise MR
- Phase 4
  - Convert 'router' to a IPC application (hide zenoh internal details)
  - Consider implementing match callbacks
  - Implement PutOptions and subscriber Sample fields
    - Support attachments
    - Support timestamping
    - Resolve how we can combine congestion control, priority and reliability settings in a coherent way to offer fewer choices at the user API layer?
      - See [discord](https://discord.com/channels/914168414178779197/940584045287460885/1311629493445853206)
    - Consider supporting sample kind (put/delete)
  - Understand the point of on_drop callback in subscriber and support it if necessary
  - Documentation cleanup
  - Unit tests
  - Lua utilities: hostname
