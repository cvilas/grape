# Troubleshooting LED matrix display framebuffer device

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