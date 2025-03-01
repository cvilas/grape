# README: app

## Brief

Application services

## Detailed Description

The `app` API implements the following features to enable application development

- Configuration file handling
- Logging
- Inter-process communication
- Signal handling

## Guidance on writing applications

- An application is a 'microservice' that implements a data processing pipeline.
- The input and output interfaces of the data pipeline are implemented as IPC endpoints (i.e. 
  publishers and subscribers). 
- The 'system' is composed of multiple such microservices running within a single compute unit 
  or across compute units spanning the local network, communicating with each other over IPC.
- Microservices are typically deployed as systemd services to manage their orchestration and 
  sequencing relative to other microservices
- See example programs for reference

TODO: 
- Drawing to show an application as a data processing pipeline with IPC endpoints, in a landscape of other applications within a host and across a network
- File organisation: 
  - data and config files
  - default, user specific and host specific locations
  - How to install them using build system
- Configuration:
  - Every pipeline has a config object
  - Facilities to install them as systemd service
- Guidance on topics: 
  - Semantically meaningful: `system/subsystem/signal` (eg: `robot00/base02/pose`, `robot00/arm01/pose`, `robot00/drill/command`)
  - Facilities to define topics (`getHostName()`, `getSystemName()`)
- Within a pipeline, avoid IPC. Make it tightly integrated