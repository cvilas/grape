# README: Camera

Cross platform camera support

- Uses [SDL3](https://github.com/libsdl-org/SDL/) for device handling and display
- Use [PipeWire](https://pipewire.org/) for concurrent access to a camera from multiple 
  applications. Set `SDL_CAMERA_DRIVER=pipewire` on the command line or as environment variable

## Implementation outline

### `camera_pub`

```                                                                 
┌──────────┐   ┌───────────┐   ┌─────────────┐   ┌───────────────┐ 
│  Camera  │─▶│ Formatter │─▶│ Compressor  │─▶│ IPC Publisher │ 
│          │   │           │   │ (LZ4)       │   │ (raw bytes)   │
└──────────┘   └───────────┘   └─────────────┘   └───────────────┘ 
```

### `camera_sub`

```
┌────────────────┐   ┌─────────────┐   ┌────────────────┐
│ IPC Subscriber │─▶│Decompressor │─▶│    Display     │
│  (raw bytes)   │   │    (LZ4)    │   │  + snapshot    |
└────────────────┘   └─────────────┘   └────────────────┘          
```

## TODO

- [ ] Integrate statistics accumulator in each processor (camera, formatter, compressor) instead of in `camera_pub` and `camera_sub`
- [ ] Introduce lockless ring buffer between compressor and publisher (main thread)
- [ ] Introduce lockless ring buffer between decompressor and display (main thread)
- [ ] Allow setting compression quality to tune overall pub-sub latency
- [ ] Allow user to specify capture resolution and fps. Let program select nearest `SDL_CameraSpec`
- [ ] Resolve discrepency between reported capture fps and actual fps (may require publish to be on a separate thread)
  - [ ] Introduce lockless ring buffer between compressor and publisher

