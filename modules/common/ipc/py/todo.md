
Step 1: Update CMakeLists.txt to Generate a Python Package

Modify your CMakeLists.txt to include the necessary steps for creating a Python package. You can use pybind11's CMake integration and add a custom target to copy the built .so file into a Python package directory.

```
find_package(pybind11 REQUIRED)

set(SOURCES
   py_bindings.cpp
)

# Build the Python module
add_library(grape_ipc_py MODULE ${SOURCES})
target_link_libraries(grape_ipc_py PRIVATE grape::ipc pybind11::module)
set_target_properties(grape_ipc_py PROPERTIES PREFIX "" OUTPUT_NAME "grape_ipc")

# Define the Python package directory
set(PYTHON_PACKAGE_DIR ${CMAKE_BINARY_DIR}/python_package)

# Copy the built module to the Python package directory
add_custom_command(TARGET grape_ipc_py POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_PACKAGE_DIR}/grape_ipc
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:grape_ipc_py> ${PYTHON_PACKAGE_DIR}/grape_ipc/
)

# Add a custom target to package the Python module
add_custom_target(package_python ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/setup.py ${PYTHON_PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/grape_ipc ${PYTHON_PACKAGE_DIR}/grape_ipc
    WORKING_DIRECTORY ${PYTHON_PACKAGE_DIR}
    COMMENT "Preparing Python package in ${PYTHON_PACKAGE_DIR}"
)
```

Step 2: Create the Python Package Structure

Organize your Python package directory. For example:

```
modules/common/ipc/py/
├── CMakeLists.txt
├── setup.py
├── grape_ipc/
│   ├── __init__.py
```

Update your setup.py to use the .so file built by CMake:

```
from setuptools import setup, find_packages
from setuptools.extension import Extension

setup(
    name="grape_ipc",
    version="0.1",
    author="Vilas Chitrakaran",
    author_email="cvilas@gmail.com",
    description="Python bindings for grape::ipc",
    long_description="",
    packages=find_packages(),
    package_data={"grape_ipc": ["grape_ipc_py.so"]},  # Include the built .so file
    zip_safe=False,
    python_requires=">=3.6",
)
```

Create an empty __init__.py file to make grape_ipc a Python package:

```
# This file can be empty or include imports for convenience
```

Step 3: Build the Python Package

Build the CMake Project:

```
mkdir -p build && cd build
cmake ..
make
```
Package the Python Module: The package_python target will prepare the Python package in the python_package directory:

```
cd python_package
python3 setup.py install
```
Step 4: Distribute the Python Package
You can now distribute the Python package by sharing the python_package directory or uploading it to PyPI. To upload to PyPI:

Install twine, build the source distribution and wheel and upload to PyPI
```
pip install twine
python3 setup.py sdist bdist_wheel
twine upload dist/*
```
Summary: 
- CMake builds the Python bindings and prepares the .so file.
- The package_python target creates a Python package directory with the .so file and setup.py.
- You can install the package locally or distribute it via PyPI.
- This approach keeps your CMake-based build system intact while enabling Python packaging for standalone use. Let me know if you need further clarification!

### Notes

1. Make sure the C++ header paths in the `setup.py` file point to the correct location of your headers.

2. If your C++ code uses C++17 features, ensure the compiler supports it (adjust the `extra_compile_args` as needed).

3. If your `init()` function has any default arguments, you might need to use `py::arg().def(value)` to specify them in the binding.