# Grape Build System (GBS)

## Introduction

GBS is a modular build system for a [monorepo](https://en.wikipedia.org/wiki/Monorepo). A monorepo is organised as a collection of software _modules_. A module is a cohesive set of

- libraries
- headers
- example programs
- unit tests
- executables
- systemd services
- documentation
- configuration files
- scripts

Modules can be fully self-contained, or depend on other modules or external projects.

### GBS features

- Configures and builds only the modules you specify, including external dependencies
- Integrates code compliance checks (linters, formatters) to help developers do things right by default
- Makes the codebase easy to extend
  - If a new feature can't be added to an existing module, just create a new one.
  - All external dependencies and their build configurations are specified in one place, in `external/CMakeLists.txt`
- Provides isolation between modules. New modules can be developed and tested without breaking the build for anybody else.
- Automates installing, packaging and uninstalling  

### How to use

- Go to the location in the `modules` sub-tree where you want the new module created.
- Call `gbs/create_module.py <module_name>` to create a new template module `module_name` at that location.
- Customise the template to your needs
  - Add source files in `src` and `include` sub-directories.
  - Add/modify library targets with `define_module_library()`
  - Declare dependencies to other modules by modifying `declare_module()`
  - Add example programs in `examples` sub-directory with `define_module_example()`
  - Add test programs in `tests` sub-directory with `define_module_test()`
  - Add executables in `apps` sub-directory with `define_nodule_app()`
  - Add documentation files in `docs` sub-directory.
  - Update the `README.md` with information relevant to other developers or users.

The build system will automatically detect the module the next time `cmake` is invoked. Specify modules to build
with `-DBUILD_MODULES=semi-colon;separated;list;of;modules` when invoking CMake during configuration. This also
automatically selects dependency modules for build.

Note that after configuration,

- `make` builds libraries and apps
- `make examples` builds all example programs
- `make tests` builds all unit tests
- `make check` builds and run tests, and generates a test report.
- `make docs` generates source code documentation using doxygen
- `make install` builds and installs library and app targets
- `<install_location>/bin/uninstall.sh` uninstalls everything

## References

- Jason Turner's [CMake Template](https://github.com/cpp-best-practices/cmake_template)
