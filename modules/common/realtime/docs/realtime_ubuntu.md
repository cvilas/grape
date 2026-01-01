# Configuring Real-time Ubuntu

## Brief

This technical note describes how to configure a Raspberry Pi 5 to isolate OS and related services 
to CPU cores [0,1]. These are called _housekeeping_ cores. This leaves CPU cores [2,3] free for 
user applications to schedule their own threads with minimal scheduling latency and jitter.

Example usage: A 'real-time' user application, using `setCpuAffinity()`, could schedule it's main 
thread (non-realtime) to run on either core 0 or 1, and one real-time thread each on cores 2 and 3.

Reference: How-to guides at <https://documentation.ubuntu.com/real-time/latest/>

## Steps

- Enable realtime kernel 
  ```bash
  pro attach
  pro enable realtime-kernel --variant=raspi
  ```
  The realtime kernel is preconfigured with options as explained [here](https://documentation.ubuntu.com/real-time/latest/reference/kernel-config-options/)
- Reboot
- Append the following to `console` line in `/boot/firmware/cmdline.txt`: 
  - `kthread_cpus=0,1`: Allocates cores 0,1 for kernel threads
  - `irqaffinity=0,1`: Lets cores 0,1 service all interrupts
  - `nohz=on nohz_full=2,3`: Disables scheduler ticks on idle CPUs, and always on cores 2,3
  - `isolcpus=2,3`: Isolates cores 2,3 from SMP load balancing and scheduling algorithms
  - `rcu_nocbs=2,3`: Offloads read-copy-update callbacks from cores 2,3
  - `rcu_nocb_poll`: Wake up RCU threads with a timer
- After the change, on my Pi5, it looks like this:
  ```bash
  $ cat /boot/firmware/cmdline.txt
  console=serial0,115200 multipath=off dwc_otg.lpm_enable=0 console=tty1 root=LABEL=writable rootfstype=ext4 rootwait fixrtc cfg80211.ieee80211_regdom=GB kthread_cpus=0,1 irqaffinity=0,1 nohz=on nohz_full=2,3 isolcpus=2,3 rcu_nocbs=2,3 rcu_nocb_poll
  ```
- Disable `irqbalance` service. This stops the OS from distributing IRQs across all available cores
  ```bash
  systemctl disable irqbalance
  systemctl stop irqbalance
  systemctl status irqbalance
  ```
- Isolate systemd services to housekeeping cores. Set `CPUAffinity=0,1` in `/etc/systemd/system.conf`. After setting, you should see this
  ```bash
  $ cat /etc/systemd/system.conf | grep CPUAffinity
  CPUAffinity=0,1
  ```
- Reboot
- Verify the parameters passed to the kernel at boot time 
  ```bash
  $ cat /proc/cmdline
  reboot=w coherent_pool=1M 8250.nr_uarts=1 pci=pcie_bus_safe  smsc95xx.macaddr=88:A2:9E:04:F4:F1 vc_mem.mem_base=0x3fc00000 vc_mem.mem_size=0x40000000  console=ttyAMA10,115200 multipath=off dwc_otg.lpm_enable=0 console=tty1 root=LABEL=writable rootfstype=ext4 rootwait fixrtc cfg80211.ieee80211_regdom=GB kthread_cpus=0,1 irqaffinity=0,1 nohz=on nohz_full=2,3 isolcpus=2,3 rcu_nocbs=2,3 rcu_nocb_poll
  ```

## Monitoring

- Monitor IRQs: `watch -n 1 cat /proc/interrupts`
- To list all IRQs associated with a given CPU, use [check_irqs.sh](../scripts/check_irqs.sh)
- List available CPUs: `cat /sys/devices/system/cpu/present`
- List isolated CPUs: `cat /sys/devices/system/cpu/isolated`
- Check if an application is running on the designated CPU cores: `ps -eo psr,tid,pid,comm,%cpu,priority,nice -T | grep <PID>`
- Snapshot for current processes and their system resource usage: `ps -A --format psr,tid,pid,comm,%cpu,priority,nice -T | sort --general-numeric-sort | grep irq`
- Generate system resource statistics: `dstat`

#### A note on `top` and process priority

`top` displays process priority in the 'PR' column in the range [-100, 39]. Lower PR mean higher process priority. PR is calculated as follows:
- For regular processes: PR = 20 + NI (NI is 'nice' in the range [-20, 19]. Pnemonic: lower NI => process is less 'nice' => takes higher priority over other processes).
- For real time processes: PR = -1 - rt_priority (rt_priority range: [1, 99])

## Further tuning

- [Tune irq affinity](https://documentation.ubuntu.com/real-time/latest/how-to/tune-irq-affinity/)
- [Isolate CPUs from general execution](https://documentation.ubuntu.com/real-time/latest/how-to/isolate-workload-cpusets/)

## Tools to measure system performance

- [How to measure maximum latency](https://documentation.ubuntu.com/real-time/latest/how-to/measure-maximum-latency/)
- [Tools for measuring real-time metrics](https://documentation.ubuntu.com/real-time/latest/reference/real-time-metrics-tools/)
 


  