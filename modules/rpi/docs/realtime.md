# Real-time Ubuntu on Raspberry Pi

## Reference

* <https://ubuntu.com/real-time>
* <https://documentation.ubuntu.com/real-time/latest/>

- Enable realtime kernel 
  ```bash
  pro attach
  pro enable realtime-kernel --variant=raspi
  # reboot
  ```

- TODO: Describe how to configure a pi to run linux on cores [1,2] and allocate cores [3,4] for RT tasks/threads
  - [modify kernel boot parameters](https://documentation.ubuntu.com/real-time/latest/how-to/modify-kernel-boot-parameters/)
  - [configure CPUs](https://documentation.ubuntu.com/real-time/latest/how-to/cpu-boot-configs/) 
    - `nohz`
    - `rcu_nocbs`
    - `isolcpus`
    - `irqaffinity`
  - [tune irq affinity](https://documentation.ubuntu.com/real-time/latest/how-to/tune-irq-affinity/)
  - [isolate CPUs from general execution](https://documentation.ubuntu.com/real-time/latest/how-to/isolate-workload-cpusets/)
  - [Write latency measurement tools](https://documentation.ubuntu.com/real-time/latest/how-to/measure-maximum-latency/)
  - [Other measurement tools](https://documentation.ubuntu.com/real-time/latest/reference/real-time-metrics-tools/)
  - [kernel boot parameters](https://documentation.ubuntu.com/real-time/latest/reference/kernel-boot-parameters/)
  - [kernel config params](https://documentation.ubuntu.com/real-time/latest/reference/kernel-config-options/)
  