# How to reduce latency over WiFi

Intel and Qualcomm Atheros WiFi modules included in most laptops support Dynamic Power Saving (DPS). When enabled (which is the default) the kernel puts the WiFi module to sleep if there is no network traffic for few milliseconds. This leads to data transport latency as high as 300ms over WiFi. To reduce average latency, turn off DPS. 

> Note that this may significantly reduce your battery life.

- Turn off DPS (Replace `wlp0s20f3` with your WiFi interface)

  ```bash
  sudo iw dev wlp0s20f3 set power_save off
  ```

- Confirm

  ```bash
  sudo iw dev wlp0s20f3 get power_save
  ```

- Make it persistent across reboots
  - Open the file `/etc/NetworkManager/conf.d/default-wifi-powersave-on.conf` for editing.
  - Change `wifi.powersave=3` (default: power management on) to `2` (power management off):

    ```toml
    [connection]
    wifi.powersave = 2
    ```

  - Restart `NetworkManager`

    ```bash
    systemctl restart NetworkManager
    ```
