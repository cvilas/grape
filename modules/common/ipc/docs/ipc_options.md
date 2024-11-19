# Message passing libraries considered

## DDS

- [FastDDS](https://github.com/eProsima/Fast-DDS)
  - Lots of experience with this; know how to make it work for complex systems. But codebase is getting bigger and more complexity is creeping in. 
  - Making shared memory + UDP work together has proven to be a challenge. 
- [cycloneDDS](https://github.com/eclipse-cyclonedds/cyclonedds) + [iceoryx](https://github.com/eclipse-iceoryx/iceoryx)
  - C, C++ and Python bindings
  - iceoryx provides zero-copy shared memory support
  - API seems too complex (too many lines of code for a hello world application)
- Problems using DDS for distributed robotics
  - Large meta-traffic due to multicast and discovery messages loads network
  - API complexity means out of the box experience for large complex systems is not good. Requires customisation of discovery service and fine-tuned QoS parameters for pub/sub

## Other

- [ZeroMQ](https://zeromq.org/)
  - Good fundamental architecture, simple API, performant, excellent documentation. 
  - Seems to be in maintenance mode. 
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
  - [Zenoh introductory webinar series](https://www.youtube.com/playlist?list=PLZDEtJusUvAY04pwmpY8uqCG5iQ7NgSrR)
  - [Genesis](https://youtu.be/ahO4kT_Zg7s) provides motivation
  - [Getting started](https://youtu.be/j8t7bV5a-qg) demonstrates what's possible with code examples
  - [ROS2 with Zenoh](https://youtu.be/9h01_MSKPS0) describes DDS limitations and how Zenoh is better for robotics
  - Written in Rust; provides binding for C, Python, REST and C++ => Good for interoperability with applications written in different languages. For instance, tooling is often in Python.
  - Enables robotics applications written in Rust; something I have been meaning to do for a while.
  - High throughput, low latency, very low overhead. See https://zenoh.io/blog/2023-03-21-zenoh-vs-mqtt-kafka-dds/
  - An alternative to MQTT, Kafka and such: messaging + distributed storage + queryable
  - Storage support is useful for logging as a fully integrated feature of communication
  - pub/sub, push/pull, pub/store/get, query/reply
  - Multiple network topology supported: p2p, brokered, routed, mesh
  - Can be used on different data links (eg: serial, bluetooth). IP is not required.
  - Indications of use in autonomous robots
  - Embeddable, even in microcontrollers
  - Very new and under active development, sponsored by Eclipse foundation.
  - Huge ambitions for the project, with plugins for variety of messaging protocols such as DDS, MQTT, etc. Also provides [ros2 RMW](https://github.com/ros2/rmw_zenoh). This could be a good thing for building large, scalable, interoperable applications. But it could also get bloated over time.
