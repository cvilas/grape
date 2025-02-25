# Ubuntu VM

## Introduction

This document describes how to set up an Ubuntu Virtual Machine using Qemu on X86-64 Debian Linux

## Install virtualization software

- Confirm your CPU supports virtualization extensions. The following should return a number above 0

  ```bash
  egrep -c '(vmx|svm)' /proc/cpuinfo
  ```

  If the above returns `0`, change bios settings to enable `VT-x` (Virtualization Technology Extension) on 
  Intel-based systems and `AMD-V` for AMD-based systems.

- Install Qemu and Virtual Machine Manager

  ```bash
  sudo apt install qemu-kvm qemu-system qemu-utils python3 python3-pip libvirt-clients \
  libvirt-daemon-system bridge-utils virtinst libvirt-daemon virt-manager -y
  ```

- Verify that `libvirtd` service is started by checking the output of the following command.  
  
  ```bash
  sudo systemctl status libvirtd.service
  ```

- Start default network and set it to auto-start on boot

  ```bash
  sudo virsh net-start default
  sudo virsh net-autostart default
  ```

- Check network status with 
  
  ```bash
  sudo virsh net-list --all
  ```

- Add user to allow access to VMs
  
  ```bash
  sudo usermod -aG libvirt $USER
  sudo usermod -aG libvirt-qemu $USER
  sudo usermod -aG kvm $USER
  sudo usermod -aG input $USER
  sudo usermod -aG disk $USER
  ```

- Reboot

## Create virtual machine

- Download latest Ubuntu _server_ ISO from [here](https://releases.ubuntu.com)

- Create VM using the above ISO
  - Let's call it `uvm`.
  - Give it at least 4 CPUs, 8G RAM, 64G disk
  - Create a user. If you do not create it at the time of initial installation, it can be done later with

    ```bash
    useradd -m <username> -s /bin/bash
    passwd <username>
    <change password>
    usermod -aG sudo <username>
    ```

  - On first boot, update the OS: `sudo apt update && sudo apt full-upgrade`  
  - Make it possible to connect by hostname: `apt install avahi-daemon`

- Set up SSH access
  - Edit `/etc/ssh/sshd_config` and set `PasswordAuthentication yes`
  - `sudo service ssh restart`
  - Connect from remote host via ssh with `ssh -A <username>@uvm.local`

- Using ssh terminal, set up the VM for building this repository by following [install](../01_install.md) instructions

## References

- [Setup Qemu in Debian Linux](https://christitus.com/vm-setup-in-linux/)
- [Virtualization - Getting Started (Fedora)](https://docs.fedoraproject.org/en-US/quick-docs/virtualization-getting-started/)
- [QEMU/KVM for absolute beginners](https://youtu.be/BgZHbCDFODk)
