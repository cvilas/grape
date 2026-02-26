# README: sense_hat

## Brief

Interface to Raspberry Pi [Sense HAT v2](https://www.raspberrypi.com/documentation/accessories/sense-hat.html#about)

## Devices

- Joystick: Supported by upstream kernel. Works with [`grape::joystick`](../../common/joystick/README.md).  
- LED matrix: Supported in the Pi OS kernel as a framebuffer device. 
- IMU: [LSM9DS1](https://www.st.com/en/mems-and-sensors/lsm9ds1.html)
- Pressure sensor: [LPS25HB](https://www.st.com/en/mems-and-sensors/lps25hb.html)
- Humidity sensor: HTS221 (discontinued. [datasheet](docs/hts221.pdf))
- Colour sensor: [TCS34725](https://ams-osram.com/products/sensor-solutions/ambient-light-color-spectral-proximity-sensors/ams-tcs34725-color-sensor)

The i2c interface on the Pi must be configured for 400kHz

```bash
# In /boot/config.txt or /boot/firmware/config.txt:
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
```

See [developer notes](./docs/developer_notes.md) for additional information

## Roadmap

- [x] Support humidity sensor
- [x] Support pressure sensor interface
- [x] Support inertial sensor interface
- [ ] Support framebuffer for LED matrix
- [ ] Report IMU data in SI units

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

## References

- [Sense HAT](https://www.raspberrypi.com/documentation/accessories/sense-hat.html#about)
- [Sense HAT on Ubuntu](https://ubuntu.com/blog/common-sense-using-the-raspberry-pi-sense-hat-on-ubuntu-impish-indri)