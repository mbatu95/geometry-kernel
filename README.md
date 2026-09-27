# geometry-kernel

`geometry-kernel` is a lightweight C++20 computational geometry kernel intended to provide reusable geometry primitives and algorithms for applications such as simulation, robotics, and CAD-like workflows.

The core currently provides header-only `Vec2` and `Vec3` types. Tests use CTest without an external testing dependency.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## OpenGL Viewer

On macOS, install GLFW with Homebrew:

```sh
brew install glfw
```

Configure and build the kernel and viewer:

```sh
cmake -S . -B build
cmake --build build
```

Run the viewer from the build directory:

```sh
./build/geometry_viewer
```

The viewer uses an OpenGL 3.3 Core context. Drag with the left mouse button to
orbit, use the mouse wheel to zoom, and press Escape to close it.

To build only the geometry kernel, its examples, and tests without searching
for GLFW or OpenGL:

```sh
cmake -S . -B build -DGEOMETRY_KERNEL_BUILD_VIEWER=OFF
cmake --build build
```