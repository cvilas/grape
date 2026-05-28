# Profiling Plot Renderer

## Prerequisites

```bash
sudo apt install linux-perf nvtop heaptrack
```

## Step 1 — CPU and energy baseline with `perf stat`

Run the application for a fixed duration and collect CPU and system energy counters:

```bash
sudo perf stat \
  -e power/energy-pkg/,power/energy-psys/ \
  -- timeout 15s ./build/native/bin/<target> 2>&1
```

## Step 2 — Real-time GPU load with `nvtop`

`nvtop` supports Intel, AMD, and NVIDIA GPUs. Run it alongside the application:

```bash
# Terminal A
./build/native/bin/<target>

# Terminal B
nvtop
```

Watch the **GPU %** bar and **power (W)** reading. A well-behaved renderer should leave
headroom on the GPU and show moderate, stable power draw.

## Step 3 — Heap profiling with `heaptrack`

`heaptrack` intercepts every `malloc`/`free` call and records the full call stack, making it ideal for finding allocation hot-paths, unexpected growth, and leaks.

```bash
heaptrack ./build/native/bin/<target>
```

Analyze on command line:

```bash
heaptrack_print heaptrack.<name>.*.zst 2>&1 | tail -10

# Top allocators by call site
heaptrack_print heaptrack.<name>.*.zst --print-allocators 2>&1 | head -60

# Peak live memory by call site
heaptrack_print heaptrack.<name>.*.zst --print-peaks 2>&1 | head -40

# Temporary allocations (freed before next alloc from same site)
heaptrack_print heaptrack.<name>.*.zst --print-temporary 2>&1 | head -40

# Leak check
heaptrack_print heaptrack.<name>.*.zst 2>&1 | grep "leaked"
```

Analyse using interactive gui

```bash
heaptrack_gui heaptrack.<name>.*.zst
```

## Rule of thumb

For a renderer to be GPU- and memory-efficient:

1. **Minimise draw calls per frame.** One large draw is always cheaper than many small ones.
2. **Keep data contiguous and pre-allocated.** Ring buffers over trees; `reserve()` at construction over per-frame growth.
3. **Decimate to screen resolution.** Never submit more vertices than there are pixels. Use min-max (not stride) to preserve peaks. Zoom compatibility comes for free.
4. **Cache static text as textures.** TTF rendering is expensive; `SDL_RenderTexture` is free.
