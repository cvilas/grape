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

Current implementation uses [eCAL](https://github.com/eclipse-ecal/ecal) backend
