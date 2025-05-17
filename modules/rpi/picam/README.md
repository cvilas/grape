# picam - Raspberry pi camera applications

## Hardware

- [Raspberry Pi 5](https://thepihut.com/products/raspberry-pi-5)
- [High Quality camera module](https://thepihut.com/products/raspberry-pi-high-quality-camera-module)
- [Camera mounting plate](https://thepihut.com/products/mounting-plate-for-high-quality-camera)
- (optional) [Mic](https://thepihut.com/products/mini-usb-microphone)
- (optional) [NVMe + PoE+ HAT](https://thepihut.com/products/hatdrive-poe-for-raspberry-pi-5)

## Host configuration

> Note: To minimise build failures, both libcamera and this project must be built with the same toolchain

Build and install libcamera
- On the RaspberryPi, build libcamera following [raspberrypi/libcamera](https://github.com/raspberrypi/libcamera)
- On all other hosts, build libcamera following [libcamera/getting started guide](https://libcamera.org/getting-started.html)

## TODO

- [ ] list
- [ ] view (libcamera source, SDL sink)
- [ ] stream (source -> encode -> pub/sub -> decode -> sink)
  - [ ] Pub supports camera settings in config file
  - [ ] Pub adds attachments to image topic: sequence number, timestamp, encoding, camera id
  - [ ] Sub listen to topic specified on command line, decodes and displays
- [ ] Confirm it works on rpi5

## References

- [libcamera-cpp-demo](https://github.com/edward-ardu/libcamera-cpp-demo)
- [rpicam-apps](https://github.com/raspberrypi/rpicam-apps)
- [CinePI](https://github.com/schoolpost/CinePI)
- [picamera2](https://github.com/raspberrypi/picamera2)
- [ustreamer](https://github.com/pikvm/ustreamer)
