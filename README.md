# Game of Life GPU

This Qt Widgets application calculates Conway's Game of Life generations on a
hardware accelerator through Microsoft C++ AMP (`<amp.h>`).

The worker selects a non-emulated Direct3D accelerator and runs each generation
with `Concurrency::parallel_for_each`. It does not silently fall back to CPU
calculation: if no compatible hardware accelerator is available, the UI reports
an error.

## Build on Windows

Use a Visual C++ toolset that still supports C++ AMP (the tested version is
MSVC 14.42), Qt 5.15, and CMake. For example, with Visual Studio 2026:

```powershell
$env:LADUGA_BUILDTOOLS = 'D:/GitHub/build-tools'
cmake -S . -B build -G "Visual Studio 18 2026" -A Win32 `
  -T v143,version=14.42.34433 `
  -DCMAKE_PREFIX_PATH="D:/GitHub/build-tools" `
  -DCMAKE_POLICY_DEFAULT_CMP0219=NEW
cmake --build build --config Release
```

Change `CMAKE_PREFIX_PATH` to the location of your Qt 5.15 installation.
Visual Studio 2026's MSVC 14.51 removed C++ AMP support, so select an older
installed toolset explicitly. A compatible Direct3D compute driver is required
for GPU execution.

To build and run the one-generation GPU regression test, enable the optional
hardware-dependent target:

```powershell
cmake -S . -B build -DGAMEOFLIFE_BUILD_GPU_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```
