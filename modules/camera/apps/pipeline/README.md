# Camera processing pipeline

## TODO

- Create Frame structure with serdes support
- Create Camera(OutputCallback)
- Create Display
- Create Encoder(OutputCallback) that serialises and LZ4 encodes
- Create Decoder(OutputCallback) that deserialises and LZ4 decodes
- Application 1: [Camera -> Encoder -> Decoder -> Display] pipeline with just callbacks
- Application 2: Camera -> Encoder -> pub
- Application 3: sub -> Decoder -> Display 

## Readme from vibe-coded example follows. To be discarded after above 

This directory contains two applications that demonstrate camera frame streaming over IPC:

## Applications

### `grape_camera_publisher`
Captures camera frames and publishes them over IPC using the grape IPC system.

**Features:**
- Uses SDL3 camera API to capture frames from available cameras
- Serializes frame data (pixels, dimensions, format, timestamp) using grape::serdes
- Publishes frames over IPC at ~30 FPS
- Displays camera specifications and frame rate statistics
- Automatically selects the first available camera

**Usage:**
```bash
# Use default topic "camera_frames"
./grape_camera_publisher

# Use custom topic
./grape_camera_publisher my_camera_topic
```

### `grape_camera_subscriber`
Subscribes to camera frames over IPC and displays them in a window.

**Features:**
- Subscribes to camera frame data over IPC
- Deserializes frame data and displays it using SDL3 renderer
- Maintains aspect ratio and centers the image in the window
- Shows frame rate statistics and latency information
- Resizable display window

**Usage:**
```bash
# Subscribe to default topic "camera_frames"
./grape_camera_subscriber

# Subscribe to custom topic
./grape_camera_subscriber my_camera_topic
```

## Usage Example

1. Start the publisher (in one terminal):
   ```bash
   ./grape_camera_publisher
   ```

2. Start the subscriber (in another terminal):
   ```bash
   ./grape_camera_subscriber
   ```

The subscriber will display a window showing the camera feed from the publisher.

## Requirements

- SDL3 library for camera capture and display
- grape::ipc for inter-process communication
- grape::serdes for frame data serialization
- A camera device connected to the system

## Frame Data Structure

The applications use a custom `Frame` structure for IPC transmission:
- `pixels`: Raw pixel data as vector of uint8_t
- `pitch`: Row stride in bytes
- `width`, `height`: Frame dimensions
- `format`: SDL pixel format (stored as uint32_t)
- `timestamp`: Capture timestamp for latency measurement

## Environment Variables

You can control the SDL camera driver by setting:
```bash
export SDL_CAMERA_DRIVER=v4l2  # Linux
./grape_camera_publisher
```

## Performance

- Publisher runs at approximately 30 FPS with built-in frame rate limiting
- Frame serialization uses a 4MB buffer to accommodate high-resolution frames
- Statistics are reported every 30 seconds for both publisher and subscriber
- Subscriber displays frame-to-frame latency information
