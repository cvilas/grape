# README: Camera

Cross platform camera support

## Design considerations

- [SDL3](https://github.com/libsdl-org/SDL/) for platform-independent device handling and display
- [PipeWire](https://pipewire.org/) on Linux for concurrent access to cameras from multiple applications. 
- Image capture and processing stages implemented following a pipeline architecture

## Notes

To make use of pipewire
- Install pipewire development packages before configuring and building this module. 
- On the terminal `export SDL_CAMERA_DRIVER=pipewire` (or add this to your terminal config script such as ~/.bashrc)


## TODO

- [ ] Define bespoke pixel_format as uint32_t fourcc codes in `ImageSpec`
- [ ] In `camera_pub`, introduce lockless ring buffer between compressor and publisher (main thread)
- [ ] Audio support for camera_pub/sub