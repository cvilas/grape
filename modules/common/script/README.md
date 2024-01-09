# README: script

## Brief

Configuration and scripting support.

## Design considerations

- Simple API to parse static configuration data organised as key-value pairs in tables
- Scripting language features:
  - Simple and minimal syntax
  - Supports math expressions and references. Example
    ```
    max_wheel_speed=100.0
    gear_ratio=10
    max_motor_speed=max_wheel_speed * gear_ratio
    reference_angle = math.pi
    ```
  - Verifiable for correctness 
  - Extendable as an application programming language for end-users to directly interact with Grape 
    applications (inspired by Roblox Studio that allow behaviors via user-defined lua scripts).

### Why Lua

It's a simple scripting language to learn, easily picked up by even school students. Yet, it is 
powerful and provides a simple API for interoperability with C/C++ in both directions; i.e. lua code
can call C/C++ functions and vice-versa.

Here's a great [Lua course](https://pikuma.com/courses/learn-lua-scripting-language-roblox) 
for those who wish to learn Lua and how to integrate Lua with C/C++.

### Why is Lua sources integrated directly into our codebase?

- Lua cannot be built like other external dependencies. It is distributed with a Makefile 
  and hardcodes compiler and build options, making it impossible to build it with different 
  toolchains or different targets. 
- As a consequence of the above, a custom CMake build script is required. For the sake of 
  consistency, sources are integrated as well, not just the build script.
- Custom build script eliminates additional external dependencies (eg: readline) and 
  per-platform idiosyncracies.
- Lua is stable and releases are infrequent. Minor releases are years apart. Therefore updating 
  Lua sources (if we ever need to) is not a major maintenance overhead.
- Lua codebase consists of a handful of C files. Easily manageable.

### Copyright considerations

Use of Lua within this project satisfies the terms and conditions of MIT license under which it 
is distributed.