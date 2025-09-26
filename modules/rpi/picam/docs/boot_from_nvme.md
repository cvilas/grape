# HowTo boot Ubuntu from NVMe drive on Raspberry Pi5 

## Preparation: Run Pi off SD card with Raspberry Pi OS

- Create a microSD card with the latest Raspberry Pi OS on it
- Boot the Pi from the SD card
- Update the OS
  ```
  sudo apt update && sudo apt full-upgrade -y
  sudo rpi-eeprom-update
  ```
- Shut down

## Install Ubuntu 24.04 on the NVMe SSD

- Connect the NVMe SSD via the PCIe ribbon cable
- Boot Pi from the microSD card
- Open terminal and type `ls /dev/nvme0`. If file exists, all is good. 
- Install Ubuntu 24.04 on the NVMe SSD
  - Open Raspberry Pi 'imager' from the Accessories menu
  - _Raspberry Pi Device_ : Select pi5
  - _Operating System_ : Navigate to 'Other general-purpose OS' -> 'Ubuntu' -> Select the latest 'Ubuntu Desktop LTS'
  - _Storage_ : Select the M.2 SSD
- Set boot order to be microSD, then SSD
  - Start configuration tool: `sudo raspi-config`
  - Navigate over to 'Advanced Options' -> 'Boot Order'
  - Select 'Boot from SD card if available, otherwise boot from NVMe or USB'
- Power down
- Remove microSD card
- Power up. The Pi should boot from the NVMe SSD

## First boot from NVMe

- Update the OS: `sudo apt update && sudo apt full-upgrade -y`  
- Make it possible to connect by hostname: `sudo apt install avahi-daemon`
- Set up SSH access
  - `sudo apt install ssh`
  - Edit `/etc/ssh/sshd_config` and set `PasswordAuthentication yes`
  - `sudo service ssh restart`
- Connect from remote host (eg: laptop) via ssh with `ssh -A <username>@<hostname>.local`
- If you are unable to clone github repositories on the Pi using ssh key forwarded from your remote host, run the following on your host
  ```
  eval `ssh-agent`
  ssh-add
  ```