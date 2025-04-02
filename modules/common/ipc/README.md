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

After evaluating [various options](./docs/ipc_options.md), current implementation settles on [eCAL](https://github.com/eclipse-ecal/ecal) backend

### 'Network' mode

By default, an IPC session restricts all its messages to within the localhost. Since the eCAL 
backend uses multicast for inter-host communications, additional configuration is required on the 
participating hosts and the network infrastructure to enable message passing across a LAN.

- In your IPC application set `scope` to `ipc::Config:Scope::Network`
- On your host
  - [Configure multicast routes](https://eclipse-ecal.github.io/ecal/latest/getting_started/cloud.html#fa-ubuntu-multicast-configuration-on-ubuntu)  
  - For TCP pub/sub, ensure hostnames are [resolvable](https://eclipse-ecal.github.io/ecal/latest/getting_started/services.html#hostname-resolution)
- You may need to configure network Switches in your LAN for ['IGMP snooping'](https://en.wikipedia.org/wiki/IGMP_snooping). Refer to your device's user manual.
