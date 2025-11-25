# picam - Raspberry pi camera applications

## Hardware

- [Raspberry Pi 5](https://thepihut.com/products/raspberry-pi-5)
- [High Quality camera module](https://thepihut.com/products/raspberry-pi-high-quality-camera-module)
- [Camera mounting plate](https://thepihut.com/products/mounting-plate-for-high-quality-camera)
- [NVMe + PoE+ HAT](https://thepihut.com/products/hatdrive-poe-for-raspberry-pi-5)
- (optional) [Mic](https://thepihut.com/products/mini-usb-microphone)

## Raspberry-pi configuration

- [Install Ubuntu](./docs/boot_from_nvme.md) on the Raspberry Pi5 NVMe SSD
- Configure the Pi5 for Grape development as per [docs/01_install.md](../../../docs/01_install.md)
- Build and install libcamera
  > Note: To minimise build failures, both libcamera and this project must be built with the same toolchain.
  - Install supporting packages and tools
    ```
    sudo apt install -y libyuv-dev libudev-dev libgnutls28-dev libssl-dev libtiff-dev libexif-dev \
    libpng-dev libjpeg-dev boost-program-options-dev libavcodec-dev meson python3-ply python3-yaml \
    i2c-tools v4l-utils libdrm-dev 
    ```
  - Build raspberry-pi fork of libcamera: [raspberrypi/libcamera](https://github.com/raspberrypi/libcamera)
- Run `sudo ldconfig`
- Build and install [rpicam-apps](https://github.com/raspberrypi/rpicam-apps)
- Run `sudo ldconfig`
- In `/boot/firmware/config.txt`, change `camera_auto_detect=1` to `camera_auto_detect=0`
  and add overlay support for the camera sensor. The following is for `imx708` on `cam0` (front, 
  i2c bus 10) connector. Use `cam1` for rear (i2c bus 11) connector 
  ```
  camera_auto_detect=0
  dtoverlay=imx708,cam0
  display_auto_detect=1
  ```
- Create udev rules to avoid the need for root permissions to access device
  - Create file: `/etc/udev/rules.d/picam.rules`
  - Add line: `SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"`
  - Reload udev rules: `sudo udevadm control --reload-rules && sudo udevadm trigger`.
  - Add user to video group: `sudo usermod -aG video $USER`
- Configure pipewire (TODO: not sure this works or is needed. Better use `libcamerify` instead)
  ```
  sudo apt install libpipewire-0.3-dev wireplumber
  systemctl --user --now enable wireplumber pipewire
  ```

## Roadmap

- [x] Create `PiCamera` class to match `grape::camera::Camera` interface
- [x] Test on Raspberry Pi camera module 3
- [x] Recreate camera_pub pipeline
- [x] Make MJPEG capture work
- [ ] Harmonise `camera_pub` and `picam_pub`
  - single common config file
  - remove code duplication
- [ ] Improve frame rate at full resolution to match `rpicam-vid`
- [ ] Make NV12 capture work on pi
- [ ] Support online configuration of camera controls (brightness, contrast, etc)

## References

- <https://github.com/NEX108/RaspberryPi5-Ubuntu24-CameraModule3>
