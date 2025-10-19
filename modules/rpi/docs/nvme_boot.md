# How to boot Ubuntu from NVMe drive on Raspberry Pi5 

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
    - Specify additional configuration to set hostname, create user name, enable ssh and enable WiFi  
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
- Connect from remote host (eg: laptop) via ssh with `ssh -A <username>@<hostname>.local`
  - If you are unable to clone github repositories on the Pi using ssh key forwarded from your remote host, run the following on your host
    ```
    eval `ssh-agent`
    ssh-add
    ```
  - Set up Pi5 for development following [install instructions](../../../docs/01_install.md)

## Additional notes

- To change host name: `hostnamectl set-hostname <hostname>`
- To add a new user
  ```bash
  useradd -m <username> -s /bin/bash
  passwd <username>
  <change password>
  usermod -aG sudo <username>
  ```
- To set up SSH access
  ```bash
  sudo apt install ssh
  sudo service ssh restart
  ```
- To enable realtime kernel, follow instructions from <https://ubuntu.com/real-time>. Basically 
  ```
  pro attach
  pro enable realtime-kernel --variant=raspi
  ```
- To enable PCIe Gen 3.0 for the NVMe drive, add the following to `/boot/firmware/config.txt`
  ```
  [pi5]
  dtparam=pciex1
  dtparam=pciex1_gen=3
  ```