# README: joystick

## Brief

Linux joystick and gamepad interface

## Design considerations

- Enumerate available devices
- Support hotplugging
- Callback on internal errors, connect/disconnect, events

## TODO

- see scratch/jsTest/evdev_joystick.c
- setAxisRange
- readAxisRange
- readAxis
- readKeysState
- readState
- thread safety
- reentrace safety
- Understand epoll and handle all error conditions
- Documentation
- Avoid exceptions everywhere. use `std::expected` and `std::optional`
- Consider moving ScopeGuard to utils
- Fix all TODOs