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

- [ ] Introduce lockless ring buffer between compressor and publisher (main thread)
- [ ] Introduce lockless ring buffer between decompressor and display (main thread)
- [ ] Allow setting compression quality to tune overall pub-sub latency
- [ ] Allow selecting nearest `SDL_CameraSpec` to what was specified
