# picam - Raspberry pi camera applications

## Hardware

- [Raspberry Pi 5](https://thepihut.com/products/raspberry-pi-5)
- [High Quality camera module](https://thepihut.com/products/raspberry-pi-high-quality-camera-module)
- [Camera mounting plate](https://thepihut.com/products/mounting-plate-for-high-quality-camera)
- (optional) [Mic](https://thepihut.com/products/mini-usb-microphone)
- (optional) [NVMe + PoE+ HAT](https://thepihut.com/products/hatdrive-poe-for-raspberry-pi-5)

## Host configuration

- [Install Ubuntu](./docs/boot_from_nvme.md) on the Raspberry Pi5 NVMe SSD
- Build and install libcamera
  > Note: To minimise build failures, both libcamera and this project must be built with the same toolchain.
  - On the RaspberryPi, build libcamera following [raspberrypi/libcamera](https://github.com/raspberrypi/libcamera)
  - On all other hosts, build libcamera following [libcamera/getting started guide](https://libcamera.org/getting-started.html)
- Configure pipewire
  ```
  sudo apt install libpipewire-0.3-dev pipewire-libcamera wireplumber libspa-0.2-modules
  systemctl --user --now enable wireplumber pipewire
  ```
- Configure the Pi5 for Grape development as per [docs/01_install.md](../../../docs/01_install.md)
- Build `camera` module. 
- TODO: The `camera` applications don't recognise the camera with pipewire support or with `libcamerify`
- TODO: `picam_list` and `picam_hello` does seem to work though

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
