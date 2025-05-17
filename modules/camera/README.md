# README: Camera

## Design requirements

- Terminal-based program on the capture side. No gui dependencies
- Capture and publish setting configurable from a file
- Minimal third party dependencies
- Cross platform

### Third party libraries

- SDL3: Cross-platform support for joysticks, cameras, audio, GPUs, window management
- gstreamer (using host libraries): Cross platform framework to build self-contained multimedia 
  pipelines (AV capture, encode, publish, record, view)

## TODO

- [x] camera_view
- [ ] Study ustreamer <https://github.com/pikvm/ustreamer>
- [ ] streamer (capture-pub/sub-view)
- [ ] Audio-Video recorder 
- [ ] Document system dependencies on the host (ffmpeg, gstreamer, x11/wayland libraries, etc)
- [ ] Note on installing and configuring pipewire as SDL_CAMREA_DRIVER for simultaneous access
