# README: Camera

Cross platform camera support

- Uses [SDL3](https://github.com/libsdl-org/SDL/) for device handling and display
- Use [PipeWire](https://pipewire.org/) for concurrent access to a camera from multiple applications. Set `SDL_CAMERA_DRIVER=pipewire` on the command line or as environment variable

## TODO

- [ ] Demo streamer over IPC (capture -> encode -> pub -> sub -> decode -> view)
