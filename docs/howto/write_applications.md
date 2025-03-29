## How to write GRAPE applications

See `app` module for demonstrative examples

## Implement 'System' as a composition of microservices

- An application is a 'microservice' that implements a data processing pipeline.
- The input and output interfaces of the data pipeline are implemented as IPC endpoints (i.e.
  publishers and subscribers).
- The 'system' is composed of multiple such microservices running within a single compute unit
  or across compute units spanning the local network, communicating with each other over IPC.
- Microservices are typically deployed as systemd services to manage their orchestration and
  sequencing relative to other microservices

![A system of microservices](./media/microservice.png)

### Why microservices?

- Enables deployment of features into production in stages, as they become available; system
capability improves over time
- Enables fault tolerance and recovery.
  - A failed service does not bring the whole system down.
  - Services can be individually controlled (started, stopped) and monitored (resource utilisation)
  - Services can be upgraded with minimal downtime (deploy binary -> restart service)
- Enables orchestration (eg: wait for network service before starting camera service)

### Data-centric architecture

Microservices follow a data-centric architecture. Applications do not explicitly connect to each
other. Instead they publish and react to data on the system (IPC) message bus. Key attributes are
connectionless publish-subscribe over topics, and loose coupling.

For a deep introduction to such design pattern, read [Layered Databus Architecture](https://github.com/cvilas/guidance/blob/main/process/lda.md).

## Configuration

Runtime configuration should be explicitly specified via configuration files or command line 
arguments. Applications should *not* depend on environment variables to operate. Rationale: 
Environment variables aren't reliably traceable and lead to issues that are hard to debug. 

- Every complex component has a `Config` data structure (see `log::Config`, `ipc::Config`).
- The `Config` data structure is populated from configuration files (written in lua) using 
  `script::ConfigScript` and `script::ConfigTable`.
- The absolute paths of configuration files are resolved using `utils::resolveFilePath()`.
- See `config` subdirectory in the `app` module for a demonstrative example of how config files are
  defined and installed using CMake

