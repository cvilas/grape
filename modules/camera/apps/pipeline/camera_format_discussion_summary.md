# Camera Format Discussion Summary

## Key Questions Discussed:
1. Why SDL_Surface images look weird after compression/decompression
2. Best pixel formats for network transmission
3. NV12 format explanation and benefits
4. Format conversion strategies for camera streaming

## Main Findings:

### Image Corruption Issues:
- **Pitch/stride mismatch** - Most common cause of weird-looking images
- **Pixel format assumptions** during compression/decompression
- **Memory layout differences** between platforms

### Optimal Network Transmission Format:
- **NV12 (recommended)**: 50% size reduction, hardware accelerated, industry standard
- **RGB24 (fallback)**: Simple, universal compatibility
- **Avoid RGBA32**: Unnecessary bandwidth overhead for camera streams

### NV12 Benefits:
- Semi-planar YUV 4:2:0 format
- 1.5 bytes per pixel vs 3 for RGB24
- Better compression characteristics
- Native GPU support on modern hardware
- Industry standard for video streaming

### Implementation Strategy:
1. Convert captured frames to NV12 before compression
2. Compress NV12 data with LZ4
3. Transmit over network
4. Keep NV12 format on receiving end
5. Use SDL_UpdateNVTexture for hardware-accelerated display

### Network Bandwidth Comparison (1920x1080 @ 30fps):
- RGBA32: ~125 MB/s
- RGB24: ~93 MB/s  
- NV12: ~48 MB/s (60% reduction!)

## Recommendation:
- Use NV12 end-to-end for optimal bandwidth and performance in camera streaming applications.
- If native capture is mjpeg, then lz4 compression is not required. This can also be transmitted directly, and will use even lower bandwidth
