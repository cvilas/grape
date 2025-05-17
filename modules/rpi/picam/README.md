# picam - Raspberry pi camera applications

## Hardware

- [Raspberry Pi 5](https://thepihut.com/products/raspberry-pi-5)
- [High Quality camera module](https://thepihut.com/products/raspberry-pi-high-quality-camera-module)
- [Camera mounting plate](https://thepihut.com/products/mounting-plate-for-high-quality-camera)
- (optional) [Mic](https://thepihut.com/products/mini-usb-microphone)
- (optional) [NVMe + PoE+ HAT](https://thepihut.com/products/hatdrive-poe-for-raspberry-pi-5)

## Host configuration

- Build and install libcamera
  - On the RaspberryPi, build libcamera following [raspberrypi/libcamera](https://github.com/raspberrypi/libcamera)
  - On all other hosts, build libcamera following [libcamera/getting started guide](https://libcamera.org/getting-started.html)

### clang/libc++ vs gcc/libstdc++

To build this project with clang/libc++ toolchain, libcamera must be built with clang/libc++ as 
well. This is how to configure libcamera for clang (libc++ is used automatically) 

```bash
export CC=clang CXX=clang++ 
meson setup build -Dcam=enabled -Dlc-compliance=disabled 
```

## TODO

- [ ] list
  - [ ] make it build with clang and gcc. Detect if libcamera was built with libstdc++ or libc++
- [ ] view
- [ ] streamer (capture-pub/sub-view)
  - [ ] Pub supports camera settings in config file
  - [ ] Pub adds attachments to image topic: sequence number, timestamp, encoding details, camera id
  - [ ] Sub listen to topic specified on command line
- [ ] Merge [grapecam](https://github.com/cvilas/grapecam) into this repo
- [ ] Confirm it works on rpi5

## References

- [cam](https://github.com/raspberrypi/libcamera/tree/main/src/apps/cam)

