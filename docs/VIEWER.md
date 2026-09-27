# OpenGL Viewer

[Back to the project README](../README.md)

`geometry_viewer` is a small optional application for visualizing Mesh objects. Its current demo creates a procedural XY grid and a box placed on the ground, then renders them with a perspective orbit camera.

## Dependencies

The viewer requires OpenGL with a 3.3 Core Profile and GLFW 3.3 or newer. On macOS, install GLFW and CMake with Homebrew:

```sh
brew install cmake glfw
```

The geometry kernel and Scene module do not depend on either graphics package.

## Build and Run

From the repository root:

```sh
cmake -S . -B build
cmake --build build
./build/geometry_viewer
```

The executable is `${build directory}/geometry_viewer`; for the commands above that is `./build/geometry_viewer`.

## Controls

- Left mouse drag: orbit around the camera target
- Mouse wheel: zoom
- Escape: close the viewer

The framebuffer resize callback updates both the OpenGL viewport and camera aspect ratio.

## Separation and Data Boundaries

The viewer depends on `geometry_scene` and `geometry_kernel`; neither lower layer depends on OpenGL. SceneObjects retain local-space, double-precision Mesh coordinates and a separate world Transform. The viewer uses the Transform as the model matrix instead of creating a transformed Mesh copy.

At upload time, the renderer converts each `Point3` coordinate from `double` to a small float GPU vertex. Mesh indices are uploaded separately and rendered with indexed drawing. Kernel `Mat4` values are row-major in storage; the renderer explicitly rearranges their elements into OpenGL column-major uniform arrays before upload. Shader files are copied to `build/viewer/shaders` by CMake, so shader loading does not depend on the process working directory.

## Current Limitations

This is intentionally a minimal viewer. It has no GUI, material system, lighting system, object picking, or scene editor. The demo uses simple uniform colors and wireframe rendering for the ground grid; the grid remains triangle geometry, so its cell diagonals are visible in wireframe mode. There is no model file import or export.