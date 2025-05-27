# README: joystick

## Brief

Linux joystick and gamepad interface

## Design considerations

- Enumerate available devices
- Support hotplugging
- Callback on internal errors, connect/disconnect, events

## TODO

- Avoid exceptions everywhere. use `std::expected` and `std::optional`
- Consider moving ScopeGuard to utils
- Fix all TODOs