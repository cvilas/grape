# README: log

## Brief

Logging services

## Features

- Lock free
- Performant (throughput comparable or better than third party solutions such as spdlog at default settings)
- Reports timestamp and source location from where the log was fired
- Threshold severity level settable at runtime
- User definable log sinks (file, network, console)
- User definable data formatters at sink
- Simple API
  ```c++
  log(severity, message);
  ```

Missing features:
- Throttling (rate-limiting) based on location  
- Tools to filter logs by severity, location   