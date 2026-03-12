# README: sense_hat

## Brief

Interface to Raspberry Pi [Sense HAT v2](https://www.raspberrypi.com/documentation/accessories/sense-hat.html#about)

## Supported devices

- Joystick: Supported by upstream kernel. Works with [`grape::joystick`](../../common/joystick/README.md).  
- LED matrix: Supported in the Pi OS kernel as a framebuffer device. 
- IMU: [LSM9DS1](https://www.st.com/en/mems-and-sensors/lsm9ds1.html)
- Pressure sensor: [LPS25HB](https://www.st.com/en/mems-and-sensors/lps25hb.html)
- Humidity sensor: [HTS221](docs/hts221.pdf)

The i2c interface on the Pi must be configured for 400kHz

```bash
# In /boot/config.txt or /boot/firmware/config.txt:
dtparam=i2c_arm=on,i2c_arm_baudrate=400000
```

## Developer notes

- [IMU](./docs/developer_notes_imu.md)
- [Display](./docs/developer_notes_fb.md)
- [Ubuntu](./docs/developer_notes_ubuntu.md)

## References

- [Sense HAT documentation](https://www.raspberrypi.com/documentation/accessories/sense-hat.html)
- [STMicroelectronics platform independent drivers](https://github.com/STMicroelectronics/STMems_Standard_C_drivers)