# gbs/py

Template files for Python bindings to C++ libraries

To select an interpreter, pass `-DPython_EXECUTABLE=/absolute/path/to/python`
when configuring CMake. When changing Python versions, use a fresh build directory
to avoid reusing extensions or wheels built for another ABI.