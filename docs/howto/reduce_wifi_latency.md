# Reduce latency over WiFi

Intel and Qualcomm Atheros WiFi modules included in most laptops support Dynamic Power Saving (DPS). When enabled (which is the default) the kernel puts the WiFi module to sleep if there is no network traffic for few milliseconds. This leads to data transport latency as high as 300ms over WiFi.

To reduce average latency, turn off DPS as follows. Note that this may significantly reduce your battery life.

- Create and open the file `/etc/NetworkManager/conf.d/wifi-powersave.conf` for editing.

- Put the following in the file and save:

  ```conf
  [connection]
  wifi.powersave = 2
  ```

  (To power management on, set powersave=3, to turn off set powersave=2)

- Restart `NetworkManager`

  ```bash
  service NetworkManager restart
  ```

- Run `iwconfig` and if it worked you should see `Power Management:off`
