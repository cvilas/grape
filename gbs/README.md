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

## GBS features

- Configures and builds only the modules you specify, including external dependencies
- Integrates code compliance checks (linters, formatters) to help developers do things right by default
- Makes the codebase easy to extend
  - If a new feature can't be added to an existing module, just create a new one.
  - All external dependencies and their build configurations are specified in one place, in `external/CMakeLists.txt`
- Provides isolation between modules. New modules can be developed and tested without breaking the build for anybody else.
- Automates installing, packaging and uninstalling

## How to use

### Creating a new module

- Go to the location in the `modules` sub-tree where you want the new module created.
- Call `gbs/create_module.py <module_name>` to create a new template module `module_name` at that location.
- Customise the template to your needs
  - Add source files in `src` and `include` sub-directories.
  - Add/modify library targets with `define_module_library()`
  - Declare dependencies to other modules by modifying `declare_module()`
  - Add example programs in `examples` sub-directory with `define_module_example()`
  - Add test programs in `tests` sub-directory with `define_module_test()`
  - Add executables in `apps` sub-directory with `define_module_app()`
  - Add documentation files in `docs` sub-directory.
  - Update the `README.md` with information relevant to other developers or users.

The build system will automatically detect the module the next time `cmake` is invoked. 

---

### Configuration options

Run CMake configuration from the repo root, passing flags with `-D<FLAG>=<VALUE>`. 

The following _module selection_ options are supported

| Flag | Type | Default | Description |
|---|---|---|---|
| `BUILD_MODULES` | `string` | _(none)_ | Semicolon-separated list of modules to build. Special value `all` builds every module. If unset, only `ALWAYS_BUILD` modules are built. |
| `DISABLE_MODULES` | `string` | _(none)_ | Semicolon-separated list of modules to forcibly exclude, even if listed in `BUILD_MODULES`. |
| `BUILD_DEPENDEES` | `BOOL` | `OFF` | When `ON`, also enables all modules that transitively depend on the modules in `BUILD_MODULES`. Only true downstream dependees are included; `ALWAYS_BUILD` modules and their sibling dependents are not pulled in. |
| `BUILD_CHANGED_MODULES` | `string` | _(none)_ | Git ref to diff against (e.g. `origin/main`). CMake runs `git diff --name-only <ref>...HEAD`, maps changed files to module names, then builds those modules plus their upstream dependencies and transitive downstream dependees. Intended for CI pipelines on merge requests: pass the destination branch and only the changed slice of the graph is built and tested. |

Example: Select `native` preset for configuration and restrict configured modules to `ipc` and its transitive dependencies and dependees:

```sh
cmake --preset native -DBUILD_MODULES=ipc -DBUILD_DEPENDEES=ON
```

The following _tooling_ options are supported

| Flag | Type | Default | Description |
|---|---|---|---|
| `ENABLE_CACHE` | `BOOL` | `ON` | Use a compiler cache (`ccache` or `sccache`) if found. |
| `ENABLE_IPO` | `BOOL` | `OFF` | Enable interprocedural (link-time) optimisation if supported. |
| `ENABLE_LINTER` | `BOOL` | `ON` | Run `clang-tidy` static analysis on every target. |
| `LINTER_WARNING_IS_ERROR` | `BOOL` | `OFF` | Treat `clang-tidy` findings as errors. |
| `ENABLE_FORMATTER` | `BOOL` | `ON` | Auto-format C/C++ sources with `clang-format` on build. |
| `ENABLE_CMAKE_FORMATTER` | `BOOL` | `ON` | Auto-format CMake files with `cmake-format` on build. |
| `ENABLE_IWYU` | `BOOL` | `OFF` | Run `include-what-you-use` analysis on every target. |
| `ENABLE_COVERAGE` | `BOOL` | `OFF` | Instrument binaries for code coverage collection. |

_Sanitisers_ are enabled at configuration time as follows. (Incompatible combinations abort configuration):

| Flag | Type | Default | Description |
|---|---|---|---|
| `ENABLE_ASAN` | `BOOL` | `OFF` | AddressSanitizer — detects memory errors. |
| `ENABLE_LSAN` | `BOOL` | `OFF` | LeakSanitizer — detects memory leaks. |
| `ENABLE_UBSAN` | `BOOL` | `OFF` | UndefinedBehaviorSanitizer — detects UB at runtime. |
| `ENABLE_TSAN` | `BOOL` | `OFF` | ThreadSanitizer — detects data races. Incompatible with ASAN/LSAN. |
| `ENABLE_MSAN` | `BOOL` | `OFF` | MemorySanitizer — detects uninitialised reads (Clang only). Incompatible with ASAN/TSAN/LSAN. |

The following _build configuration_ options are supported

| Flag | Type | Default | Description |
|---|---|---|---|
| `CMAKE_BUILD_TYPE` | `string` | `RelWithDebInfo` | Build type: `Debug`, `Release`, `MinSizeRel`, `RelWithDebInfo`. |
| `BUILD_SHARED_LIBS` | `BOOL` | `ON` | Build shared libraries (`.so`). Set `OFF` for static. |
| `CMAKE_INSTALL_PREFIX` | `path` | `/opt/<project>` | Installation root used by `make install`. |


## Building

After configuration, targets are built with `cmake --build <build/dir>`. The following _targets_ are supported:

| Target | Description |
|---|---|
| `all` | Build all libraries and app executables in enabled modules. |
| `examples` | Build all example programs. |
| `tests` | Build all unit test binaries. |
| `check` | Build and run all unit tests; generate a test report. |
| `format` | Apply `clang-format` and `cmake-format` to all sources in enabled modules. |
| `docs` | Generate Doxygen HTML documentation into `build/docs/`. |
| `install` | Build and install libraries, headers, and apps under `CMAKE_INSTALL_PREFIX`. |
| `pack` | Package installed artifacts for deployment. |

Example 1: Build all targets and run unit tests:

```sh
cmake --build build/native --parallel --target all examples check
```

Example 2: Build version check target:

```sh
cmake --build build/native --parallel --target grape_show_version
```

## Uninstalling

Run `<install_prefix>/bin/uninstall.sh`.

