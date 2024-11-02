# Usage patterns described with examples

## Core concepts

- Zenoh is a _distributed messaging system_ that operates on _key/value_ spaces. The core concepts behind the protocol are [described here](https://zenoh.io/docs/manual/abstractions/).
- The supported network topologies are [described here](https://zenoh.io/docs/getting-started/deployment/).

## Discovery

- `scout`: The process of discovering Zenoh applications is called scouting. This example prints details of endpoints visible to a Zenoh session.
- `info`: A simpler example that just prints the IDs for the session and the other routers and peers visible to it.
- `liveliness_declare`, `liveliness_get`, `liveliness_sub`: Demonstrates how to enable liveliness tracking on a key expression. This feature enables automatic notification when entities, endpoints or services of interest come alive or drop away.

## Latency and throughput

- `ping`, `pong`: Performs roundtrip latency measurement between a pair of endpoints.
- `throughput_pub`, `throughput_sub`: Performs throughput measurement between a pair of endpoints.

## pub/sub

- `put`, `pub`, `pub_delete`: Demonstrates a couple of different ways to publish data values over keys. The `put` example shows how a session can directly put data on the bus without a publisher. The `delete` example shows how to notify subscribers to delete previously published data. Note that data encoding can be specified within `PublisherPutOptions` for every publish event, enabling publishers to post different data types over the same key. The subscribers receive encoding information with the data sample.
- `sub`, `pull`: Demonstrates different ways to receive data from matched publishers. The receiver can either poll for the latest data (`pull`) or receive them as they become available (`sub`) via a callback.
- `pub_cache`, `query_sub`: Demonstrates a publisher with historical storage, and how a subscriber can retrieve that history of past publications on startup.
- `attachment_pub`, `attachment_sub`: Demonstrates how to modify the publisher and subscriber to transport attachments or auxilliary payloads with the main data payload.

### Shared memory transport

- `pub_shm`,`sub`: Demonstrates how shared-memory can be used to transport data between endpoints. Shared-memory transport is used only if both the publisher and the subscriber are on the same host and are configured to use shared-memory. When on different hosts, they automatically fallback to using the network transport layer. Additionally, with shared-memory enabled, the publisher still uses the network transport layer to notify subscribers of the shared-memory segment to read. Therefore, for very small messages, shared-memory transport could be less efficient than using the default network transport to directly carry the payload. This also means that the key benefit of using shared-memory is to improve data throughput and not latency.

## Remote procedure call

- `queryable`, `query_get`, `query_get_channel`, `get_channel_non_blocking`: Demonstrates how to instantiate a service (`queryable`) that waits for and replies to remote requests. The endpoints requesting the service can process the response asynchronously via callback (`get`), or synchronously (`get_channel`, `get_channel_non_blocking`). For more on channels [see here](https://zenoh-cpp.readthedocs.io/en/latest/channels.html).

## Brokered and routed communications

The default peer-to-peer communication strategy may be undesirable in large distributed systems of systems for two reasons: (a) It may not scale well due to the network overhead required to track the state of all entities, and (b) it may not provide sufficient isolation between endpoints. Additionally, endpoints running on resource-constrained devices may not provide the underlying transport mechanisms required for efficient peer-to-peer communication. In such instances, endpoints can be configured to operate in `client` mode where they maintain a single session with a router which arbitrates connectivity amongst all other clients in the system and also routes data traffic.

Note that routers can also be used with nodes operating as `peers`. In this case 'gossip' scouting can be used and multicast scouting turned off to improve network efficiency and scalability. For more on scouting mechanism, see [deployment patterns](https://zenoh.io/docs/getting-started/deployment/) for zenoh applications.

- `router`, `pub_client`, `sub_client`: Demonstrates how to instantiate a router and clients that associate with it. Although not in the `router` example, a session configured as router can have publishers and subscribers just like any other.

## Configuration

- All parameters available for configuring a Zenoh session are described in this [default configuration file](https://github.com/eclipse-zenoh/zenoh/blob/master/DEFAULT_CONFIG.json5).
- Some of the examples above demonstrate how the parameters can be set/modified. For instance,
  - `pub_shm` enables shared-memory transport
  - `pub_cache` enables timestamping
- There is more on configuration [here](https://zenoh.io/docs/manual/configuration/)

## Introspection tools

- Use [wireshark](https://www.wireshark.org/). There is a [plugin](https://github.com/ZettaScaleLabs/zenoh-dissector) available to decode zenoh traffic.

## Zenoh for large distributed autonomous systems

### Example application

Remote monitoring and management of fleets of logistics robots operating in multiple geographically distributed warehouses.

### System architecture

Use [Layered Databus Architecture](https://github.com/cvilas/guidance/blob/main/process/lda.md). Note that all subsystems use a single messaging backbone and protocol, but the system is logically partitioned into machine databuses, site databus (fleet management services), public network and so on.

### Configuration example

![sample architecture](../docs/media/sample_arch.png)

- Routers assist with discovery of nodes within and across hosts.
  - Within a host, routers only forward discovery information and not data traffic.
  - Across hosts that are on different networks, routers additionally forward data traffic.
- Nodes within a host are typically configured in `peer` mode. This enables them to directly connect with each other for data transport. 
- Multicast scouting can be disabled across the entire system, relying instead on 'gossip' scouting and routing for discovery. This reduces discovery traffic overhead, improving scalability.
- Devices on the public network are configured as `client`s to the public routers.
- Liveliness tokens are used with key-expression matching to auto discover services as they come online.
