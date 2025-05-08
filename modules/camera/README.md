# README: Camera

## Design requirements

### capture -> pub/sub -> view

- Terminal-based program on the capture side. No gui dependencies
- Capture and publish setting configurable from a file
- Minimal third party dependencies
- Cross platform

### Thoughts on third party dependencies

- Camera capture: SDL3 (alternative: gstreamer)
- Window management: SDL3 (alternative: GLFW)
- Full multimedia pipeline (capture, encode, publish): gstreamer (use host system libraries)
- Why SDL3: powerful base to develop any multimedia application, GPU support, I/O support
- Why gstreamer: powerful, industry relevant skill

## TODO

- [x] camera_view
- [ ] streamer (capture-pub/sub-view)
  - [ ] Study ustreamer <https://github.com/pikvm/ustreamer>
- [ ] Audio-Video recorder 
  - Build ffmpeg from source
  - Encode both video and audio 
- [ ] Merge [grapecam](https://github.com/cvilas/grapecam) into this repo
- [ ] Document system dependencies on the host (ffmpeg, gstreamer, x11/wayland libraries, etc)
- [ ] Confirm it works on rpi5
- [ ] Remove the need to apply THIRD_PARTY_COMPILER_WARNINGS

