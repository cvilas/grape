# Profiling Plot Renderer

## Prerequisites

```bash
sudo apt install linux-perf nvtop heaptrack
```

## CPU and energy baseline with `perf stat`

Run the application for a fixed duration and collect CPU and system energy counters:

```bash
sudo perf stat -e power/energy-pkg/,power/energy-psys/ \
  -- timeout 15s </path/to/grape_plot_example> 2>&1
```

## Real-time GPU load with `nvtop`

`nvtop` supports Intel, AMD, and NVIDIA GPUs. Run it alongside the application:

```bash
</path/to/grape_plot_example>

# On another terminal
nvtop
```

Watch the `GPU %` and `power (W)` reading. A well-behaved renderer should leave headroom on the GPU and show moderate, stable power draw.

## Heap profiling with `heaptrack`

`heaptrack` intercepts `malloc`/`free` calls and records the full call stack, making it ideal for finding allocation hot-paths, unexpected growth, and leaks.

```bash
# capture data
heaptrack </path/to/grape_plot_example>

# Analyse using interactive gui:
heaptrack_gui heaptrack.<name>.*.zst
```

