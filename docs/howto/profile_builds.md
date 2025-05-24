# How to profile builds

To profile the build process and determine bottlenecks:

* Configure to use `Ninja` as the build system generator
* Build. The profiling information gets logged in `.ninja_log` in the build directory
* Process `.ninja_log` into a json trace file using [`ninjatracing`](https://github.com/nico/ninjatracing)
* View the trace file in [Perfetto](https://ui.perfetto.dev/)