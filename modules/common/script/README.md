# README: script

## Brief

Configuration and scripting support using [Lua](https://www.lua.org/).

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
    applications (inspired by Roblox Studio that allow behaviors via user-defined Lua scripts).

### Why Lua

- Simple to learn.  
- Powerful fully featured programming language
- Provides simple API for C/C++ interoperability in both directions; i.e. Lua code can call C/C++
  functions and vice-versa.

Here's a good [Lua course](https://pikuma.com/courses/learn-lua-scripting-language-roblox) 
for those who wish to learn Lua and how to integrate Lua with C/C++.

Lua is integrated directly into the module (not as an external project), for the following reasons:

- Lua is distributed with a Makefile that hardcodes compiler and build options, making it 
  impossible to build with different toolchains. A custom CMake build script is provided to 
  overcome this problem. For consistency, it makes sense to integrare sources with the build script.
- Custom build script eliminates external dependencies (eg: readline) and platform idiosyncracies.
- Lua releases are infrequent. Minor releases are years apart. The codebase consists of a handful 
  of C files. Therefore the maintenance overhead is negligible.

### Copyright considerations

Use of Lua within this project satisfies the terms and conditions of MIT license under which it 
is distributed.