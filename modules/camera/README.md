# README: Camera

Cross platform camera support

- Uses [SDL3](https://github.com/libsdl-org/SDL/) for device handling and display
- Use [PipeWire](https://pipewire.org/) for concurrent access to cameras from multiple applications. 
  To make use of pipewire, you must
  - Install pipewire development packages before building SDL3. 
  - On your terminal `export SDL_CAMERA_DRIVER=pipewire` (or add this to your terminal config script such as ~/.bashrc)


## TODO

- [ ] Integrate statistics accumulator in each processor (camera, formatter, compressor) instead of in `camera_pub` and `camera_sub`
- [ ] Introduce lockless ring buffer between compressor and publisher (main thread)
- [ ] Experimentally determine and then document how to improve network performance
  - [ ] Set UDP buffer sizes as required in eCAL config (ecal.yaml)
  - [ ] Increase system socket buffer sizes in /etc/sysctl.conf (net.core.rmem_max/wmem_max/rmem_default/wmem_default)