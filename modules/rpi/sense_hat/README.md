# README: sense_hat

## Brief

Interface to Raspberry Pi [Sense HAT](https://www.raspberrypi.com/documentation/accessories/sense-hat.html#about)

## Devices

- Joystick: Supported by upstream kernel. Works with [`grape::joystick`](../../common/joystick/README.md).  
- LED matrix: Supported in the Pi OS kernel as a framebuffer device. 
- IMU: [LSM9DS1](https://www.st.com/en/mems-and-sensors/lsm9ds1.html)
- Pressure sensor: [LPS25HB](https://www.st.com/en/mems-and-sensors/lps25hb.html)
- Temperature and humidity sensor: HTS221
- Colour sensor: [TCS34725](https://ams-osram.com/products/sensor-solutions/ambient-light-color-spectral-proximity-sensors/ams-tcs34725-color-sensor)

## TODO

- [ ] Review SenseHat py [API](https://sense-hat.readthedocs.io/en/latest/api/)
- [ ] Review [underlying C implementation](https://github.com/RPi-Distro/RTIMULib/) of the py interfaces
- [ ] Support polling interface for temperature, humidity, pressure and colour sensors. 
  - [ ] Write examples similar to sense hat book.
- [ ] Add framebuffer support for LED matrix. 
  - [ ] Write examples similar to sense hat book
- [ ] Support reading IMU at high/native data rates (trigger callback)

## Troubleshooting

- Framebuffer devices
  ```bash
  # list framebuffer devices
  cat /proc/fb

  # show properties
  fbset -i -fb /dev/fb0

  # show sysfs attributes  
  cat /sys/class/graphics/fb0/name
  cat /sys/class/graphics/fb0/modes
  cat /sys/class/graphics/fb0/virtual_size
  cat /sys/class/graphics/fb0/bits_per_pixel
  ```
- i2c devices
  ```bash
  # List all I2C buses
  i2cdetect -l
  
  # Scan a specific bus for devices (e.g., bus 1)
  i2cdetect -y 1

  # Get info on specific device
  cat /sys/class/i2c-dev/i2c-1/device/1-001c/uevent # magnetometer
  ```
## References

- [Sense HAT](https://www.raspberrypi.com/documentation/accessories/sense-hat.html#about)
- [Sense HAT on Ubuntu](https://ubuntu.com/blog/common-sense-using-the-raspberry-pi-sense-hat-on-ubuntu-impish-indri)