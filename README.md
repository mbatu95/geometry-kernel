# geometry-kernel

`geometry-kernel` is a lightweight C++20 computational geometry kernel intended to provide reusable geometry primitives and algorithms for applications such as simulation, robotics, and CAD-like workflows.

The core currently provides header-only `Vec2` and `Vec3` types. Tests use CTest without an external testing dependency.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```