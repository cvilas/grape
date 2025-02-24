# README: app

## Brief

GRAPE application services

## Detailed Description

Provides a framework for writing GRAPE applications. The following features are integrated

- Configuration file handling
- Logging
- Inter-process communication
- Signal handling

See example programs for reference

## A note on how to write GRAPE applications

The `grape::app` API provides initialisation, configuration, logging and IPC services. This is all 
that one typically needs to write an application program that encapsulates a data processing 
pipeline. The inputs to and outputs from the pipeline are implemented as IPC endpoints (i.e. 
publishers and subscribers). Therefore, applications can run as independent processes within a PC 
and/or compute units spanning the local network, communicating with each other over IPC. These 
processes may be configured to start at boot time as systemd services.
