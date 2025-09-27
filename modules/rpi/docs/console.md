# Console over USB on Raspberry Pi

(This assumes that the Pi boots from an SD card containing Rasbian OS)

- Unplug the boot SD card from the Pi and plug it into your Linux PC
- Mount the `bootfs` partition on the SD card
- Open `config.txt` and add the following line at the bottom
  ```
  dtoverlay=dwc2
  ```
- Open `cmdline.txt` and add the following after 'rootwait'
  ```
  modules-load=dwc2,g_ether
  ```
  On my Raspberry Pi Zero 2W, this is what the whole text now looks like
  ```
  console=serial0,115200 console=tty1 root=PARTUUID=f127a59b-02 rootfstype=ext4 fsck.repair=yes rootwait modules-load=dwc2,g_ether quiet splash plymouth.ignore-serial-consoles
  ```
- In the same directory, create an empty text file called `ssh`. This will enable ssh on _first_ boot (but not subsequent boots!)
- Unmount the SD card, plug it back into the Pi, boot it up
- Connect your Linux PC to the Pi using a USB cable. The Pi will be detected as a RNDIS/Ethernet gadget
- On the terminal, connect to pi using `ssh pi@raspberrypi.local`

## References

- https://youtu.be/xj3MPmJhAPU?si=ry7H-M_i-KTkdb6u
- https://learn.adafruit.com/turning-your-raspberry-pi-zero-into-a-usb-gadget/serial-gadget