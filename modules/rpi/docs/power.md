# Power supply for a Raspberry Pi running Ubuntu

The Pi must be powered by a USB-PD power supply that is able to deliver 5A at 5V. Without it, 
downstream USB ports are restricted in their current output and the OS _feels_ throttled, going by 
sluggishness in the UI.

To remove the restriction on USB max current:
- Add the line `usb_max_current_enable=1` to `/boot/firmware/config.txt`.
- Force Pi to assume a 5A supply with
  - `sudo rpi-eeprom-config --edit` 
  - Add `PSU_MAX_CURRENT=5000` .

*Note:* Only do this if the power supply is capable of delivering 5A @ 5V, or you are risking damage.