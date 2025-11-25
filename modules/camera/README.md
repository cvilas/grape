# README: Camera

Cross platform camera support

- Uses [SDL3](https://github.com/libsdl-org/SDL/) for device handling and display
- Use [PipeWire](https://pipewire.org/) for concurrent access to cameras from multiple applications. 
  To make use of pipewire, you must
  - Install pipewire development packages before building SDL3. 
  - On your terminal `export SDL_CAMERA_DRIVER=pipewire` (or add this to your terminal config script such as ~/.bashrc)


## TODO

- [ ] Define custom `ImageSize`, `PixelFormat` (as uint32_t fourcc codes), `ImageSpec`
- [ ] Integrate `ImageSpec` into `ImageFrame`
- [ ] Harmonise `Camera` and `PiCamera` interfaces and behaviours
- [ ] In `camera_pub`, introduce lockless ring buffer between compressor and publisher (main thread)
