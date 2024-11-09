# Message passing libraries considered

- [FastDDS](https://github.com/eProsima/Fast-DDS)
  - Lots of experience with this; know how to make it work for complex systems
  - Codebase is getting bigger and more complex
  - Making shared memory + UDP work together has proven to be a challenge
  - High network load due to large meta-traffic due to multicast and discovery messages
  - Requires lots of customisation of discovery service and fine-tuned QoS parameters for pub/sub
- [ZeroMQ](https://zeromq.org/)
  - Good fundamental architecture, simple API, performant, excellent documentation. 
  - Seems to be in maintenance mode
- [nanomsg](https://github.com/nanomsg/nanomsg)
  - From the authors of ZeroMQ. 
  - Claims to [fix architectural oversights in ZeroMQ](https://nanomsg.org/documentation-zeromq.html)
  - In maintenance mode. Recommends using `nng` instead.
- [nng](https://github.com/nanomsg/nng)
  - 'Spiritual successor' to nanomsg, retains API compatibility.
  - Written in C, so usable in space constrained embedded systems
  - Simple API
  - Communication patterns: pub/sub, req/rep, bus (many-to-many), pair, push/pull, survey (query many)
  - Protocols: inproc, interproc, TCP
  - Paid support is available
- [bloomberg/blazingmq](https://github.com/bloomberg/blazingmq)
  - Alternative to Kafka
  - C++, Java and Python clients libraries
- [Zenoh](https://github.com/eclipse-zenoh/zenoh)
  - Written in Rust; provides binding for C, Python, REST and C++
  - Requires Rust toolchain to build, on top of C++ toolchain. Adds to complexity
  - High throughput, low latency, very low overhead. See https://zenoh.io/blog/2023-03-21-zenoh-vs-mqtt-kafka-dds/
  - An alternative to MQTT, Kafka and such: pub/sub messaging + distributed storage + queryable
  - Storage support is useful for logging as a fully integrated feature of communication
  - Multiple network topology supported: p2p, brokered, routed, mesh
  - Can be used on different data links (eg: serial, bluetooth). IP is not required.
  - Indications of use in autonomous robots
  - Suitable for large distributed systems spanning microcontrollers to WAN
  - Very new and under active development, sponsored by Eclipse foundation.
- [eCAL](https://eclipse-ecal.github.io/ecal/stable/index.html)
  - Builtin HDF5 data record/replay. Probably eliminates the need to build my own
  - Very simple API: pub/sub/query. No complex configuration
  - Lots of tooling (In Qt, but we can choose not to build them)
  - Depends on `protobuf` internally, but seems to cross compile just fine
  - Scalability should be better than DDS due to multicast instead of unicast
  - Good for distributed systems across a LAN, but not WAN

eCAL and Zenoh are currently the two most promising candidates. See a comparison between the two 
[here](https://github.com/eclipse-ecal/ecal/discussions/1847). eCAL is more mature and simpler of 
the two.