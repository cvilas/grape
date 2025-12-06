# ahrs - Attitude-Heading Reference System using Raspberry Pi5 Sense HAT

- `sudo apt install sense-emu-tools sense-hat geany`
- Add the following udev rules to `/etc/udev/rules.d/sensehat.rules`
```
SUBSYSTEM=="i2c-dev", KERNEL=="i2c-[0123456]", GROUP="plugdev", MODE="0660"
SUBSYSTEM=="input", ENV{LIBINPUT_DEVICE_GROUP}=="*:rpi-sense-joy", GROUP="plugdev", MODE="0660"
SUBSYSTEM=="graphics", ENV{ID_PATH}=="*-rpi-sense-fb", GROUP="plugdev", MODE="0660"
```
- Apply rules: `sudo udevadm control --reload-rules && sudo udevadm trigger`

## TODO

- [ ] Make the hardware work: https://ubuntu.com/blog/common-sense-using-the-raspberry-pi-sense-hat-on-ubuntu-impish-indri
  - ref:  https://waldorf.waveform.org.uk/2021/common-sense-hat.html
  - Get source code directly from STM pages for the devices
- [ ] Coordinate-frame aware linear algebra [refx](https://github.com/mosaico-labs/refx), [PoC](https://github.com/cvilas/scratch/blob/master/linalg.cpp)
- [ ] constexpr matrix and quaternions operations to support AHRS implementation
- [ ] 3D viewing and signal plotting (SDL3, imgui, implot)