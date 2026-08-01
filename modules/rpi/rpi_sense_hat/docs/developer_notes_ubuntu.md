# Sense HAT on Ubuntu


To use the Sense HAT as regular user (i.e. without sudo), create and add the following udev rules to file `/etc/udev/rules.d/99-user-sense.rules`:

```bash
SUBSYSTEM=="i2c-dev", KERNEL=="i2c-[0123456]", GROUP="plugdev", MODE="0660"
SUBSYSTEM=="input", ENV{LIBINPUT_DEVICE_GROUP}=="*:rpi-sense-joy", GROUP="plugdev", MODE="0660"
SUBSYSTEM=="graphics", ENV{ID_PATH}=="*-rpi-sense-fb", GROUP="plugdev", MODE="0660"
```

Reboot for this to take effect.

## References

- [Sense HAT on Ubuntu](https://ubuntu.com/blog/common-sense-using-the-raspberry-pi-sense-hat-on-ubuntu-impish-indri)
