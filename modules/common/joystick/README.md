# README: joystick

## Brief

Linux joystick and gamepad interface

## Design considerations

- Enumerates available devices
- Supports hotplugging
- Triggers callback to report internal errors, connect/disconnect events, and input events

## Limitations

- Axis normalised to [-1,1] by default. This may be suboptimal for some use-cases
- No method to set axis deadzone and fuzz
- Fix on ctrl+c: ` Failed on epoll_wait: Interrupted system call`
- Consider moving ScopeGuard to utils
