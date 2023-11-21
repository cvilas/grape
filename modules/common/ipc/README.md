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
- Supports multiple languages: atleast C++ and Python.

## Zenoh

Zenoh was chosen after considering a [few options](./docs/ipc_options.md).

[Zenoh](https://zenoh.io/docs/overview/what-is-zenoh/) is a new protocol under active development that meets all the above requirements with less hidden overheads compared to protocols such as DDS. Capabilities of Zenoh are best explained with examples. So head [over there](./examples/README.md).

Zenoh API and data structures are directly used in grape and not abstracted away. This is because:

- There are no good alternatives to Zenoh at the moment. The IPC backend is unlikely to change soon. Therefore, there are no advantages to abstracting away the interface and hiding the implementation details.
- Zenoh is still new and evolving. API is likely to change, and new features are likely to be implemented. Providing an abstraction layer at this time would add maintenance and development overheads to keep it up to date.

The decision to provide an abstract interface is deferred until API complexity or feature creep necessitates it.

### Copyright

Use of Zenoh within this project satisfies the terms and conditions of Apache License version 2.0 under which it is distributed.
