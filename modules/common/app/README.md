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

A GRAPE application

- Encapsulates a data processing pipeline as a 'service', with inputs to and outputs from the pipeline implemented as IPC endpoints
- Exploits the `grape::app` API for initialisation, configuration, logging and IPC
- Services run as independent processes distributed across CPUs and communicate with each other over IPC
- Services may be configured to start at system boot (using systemd)  