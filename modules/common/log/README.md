# README: log

## Brief

Logging services

## Features

- Usable in low-latency and realtime contexts
  - Lock free
  - No memory allocation
- Performant (throughput comparable or better than third party solutions such as spdlog at default settings)
- Reports timestamp and source location from where the log was fired
- Threshold severity level settable at runtime
- User definable log sinks (file, network, console)
- User definable data format at sink
- Simple API
  ```c++
  log(severity, message);
  ```

Missing features:
- Throttling (rate-limiting) based on location  
- Tools to filter logs by severity, location   