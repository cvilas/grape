# README: joystick

## Brief

Linux joystick and gamepad interface

## Design considerations

- Enumerates available devices
- Supports hotplugging
- Triggers callback to report internal errors, connect/disconnect events, and input events

## Limitations

- No method to set axis deadzone and fuzz

## Troubleshooting

- Run `evtest` to detect all event devices. 
- Run `ls -l /dev/input/by-path` to list all detected event devices 