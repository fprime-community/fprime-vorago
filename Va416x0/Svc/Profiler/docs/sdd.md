# `Va416x0Svc::Profiler`

The `Profiler` component can be included in deployments to gather fine-grained
profiling data on the execution of individual functions. It leverages the
built-in `clang` instrumentation provided by the `-finstrument-functions-*`
flags and defines efficient hooks to maintain a lean footprint.

## Configuration

The `Profiler` component should be included by using the
[`va416x0-baremetal-profiler.cmake`](../../../../cmake/toolchain/va416x0-baremetal-profiler.cmake)
CMake toolchain to build your project. This adds the instrumentation flags to
the compiler commands and defines the `VA416X0_ENABLE_PROFILER` macro (which
should be used to identify profiler-specific code sections). An instance of the
`Va416x0Svc::Profiler` component can then be included in your topology.

By default, all functions in the topology (including auto-coded functions) are
instrumented. Functions can be excluded from instrumentation using one or more
of the following flags:
- `-finstrument-functions-exclude-file-list`: A comma-separated list of file paths. This can be a fully-specified path or any partial portion (note: do not use `.`, `..`, `~`, or any environment variables as these values are not expanded). Substring matches are performed and the function is excluded if any portion of the path for its source file match any items in the list.
- `-finstrument-functions-exclude-function-list`: A comma-separated list of function names. These should be de-mangled names for C++ functions. The function is excluded if its name matches any items in the list.

The `Profiler` component stores profile data in a configurable memory region.
The address and size of this memory region are configured in
[`config/ProfilerCfg.hpp`](../../../../config/ProfilerCfg.hpp). These values
are set to 0 by default so they must be overridden in a project-specific
configuration file.

```cpp
//! Starting address of the memory region used to store profiler data
//! NOTE: this must be overridden when using the Profiler
constexpr U32 PROFILER_MEMORY_REGION_START = 0;

//! Size of the memory region used to store profiler data, in bytes
//! NOTE: this must be overridden when using the Profiler
constexpr U32 PROFILER_MEMORY_REGION_SIZE = 0;
```

## Usage

The `Profiler` component is designed as a standard F-Prime component which
should be included in the target deployment.

The `Profiler` component defines the `Profiler.ENABLE` command which is used to
trigger a profile capture. Once the command is sent, the profiler will begin to
log function entry/exit events from instrumented functions which are called
until the configured data region fills up.

Once the profile has been captured, the events must be extracted from the data
region. The implementation of this is left up to the discretion of the user.
See [Data Format](#data-format) for more details on the binary format of
profile events.

## Data Format

Each profile event comprises 8 bytes (2 `U32` values) and corresponds to the
entry to or exit from an instrumented function.
1. The first `U32` encodes the phase (entry vs. exit) and function address. Bit 31 stores the phase (0 for function entry and 1 for function exit) and bits 0 thru 30 store the function address (note that this is stored in Thumb mode so bit 0 should be masked out before comparing to the symbol table).
1. The second `U32` stores the timestamp for the event. This is read from the Va416x0 SYST_CVR register which is a counter that starts at 0xFFFFFF (when the profiler is enabled), decrements once per clock cycle, and wraps around when it reaches 0.

```
63      62                         31                                  0
 +-------+--------------------------+----------------------------------+
 | Phase | Function Address (Thumb) |        Timestamp (cycles)        |
 +-------+--------------------------+----------------------------------+
```

## Idiosyncracies

- The profiler will not work correct for any function address that have bit 31 set since it uses bit 31 to store the phase for each event. See [Data Format](#data-format) for more details.
