# Architecture

[Back to the project README](../README.md)

The project separates mathematical geometry, object organization, and visualization into independent layers. Dependencies point toward the geometry kernel:

```text
Application
    │
    ▼
geometry_scene
    │
    ▼
geometry_kernel

geometry_viewer (optional)
    │
    ▼
geometry_scene
    │
    ▼
geometry_kernel
```

## Dependency Direction

- `geometry_kernel` has no dependency on Scene, OpenGL, or GLFW.
- `geometry_scene` depends on the kernel for Mesh, Transform, and AABB types. It has no graphics dependency.
- `geometry_viewer` depends on Scene and the kernel, as well as GLFW and OpenGL.
- Applications can depend on the reusable layers without requiring the viewer.

The CMake `GEOMETRY_KERNEL_BUILD_VIEWER` option controls whether viewer dependencies are discovered and the `geometry_viewer` target is built. It defaults to on for a top-level checkout and off when included as a subproject; callers can explicitly set either value. The `geometry_scene` target remains available when the viewer is disabled.

## Geometry Kernel

`geometry_kernel` contains the mathematical and geometric foundation: vectors, points, matrices, affine transforms, rays, planes, triangles, AABBs, indexed triangle meshes, procedural mesh generators, and intersection queries. The current API is header-only under `include/geometry`.

The kernel does not own rendering state or depend on a graphics API. Geometry tests can run without creating a window or graphics context.

## Scene

`geometry_scene` is a small organization layer under `include/scene`. A `SceneObject` contains a unique non-empty name, a local-space `geometry::Mesh`, a `geometry::Transform`, and visibility state. `Scene` stores a flat collection of these objects, supports lookup and removal by name, and computes bounds over visible objects.

There is no hierarchy, parent-child relationship, or component system. Scene has no OpenGL types or rendering behavior.

## Viewer

`geometry_viewer` is an optional application under `viewer/`. It builds a Scene from procedural geometry, uploads indexed meshes to GPU buffers, and draws visible objects with their object transforms. GLFW owns window creation, input, and context setup; the renderer and shaders use OpenGL.

The viewer is not required by the kernel or Scene targets. Disabling it avoids searching for OpenGL and GLFW packages.

## Why Rendering Is Separate

Keeping rendering outside geometry allows kernel types and tests to remain usable in programs that do not create a window or use a GPU. Rendering-specific representations and conversions belong at the viewer boundary; the kernel keeps its double-precision coordinates and matrix convention.

## Local Geometry and World Placement

A SceneObject retains its Mesh in local space. Its Transform describes placement in world space. Moving or rotating an object updates the Transform rather than rewriting the Mesh vertices. The viewer uploads the local Mesh and supplies the Transform as the model matrix when drawing.

World-space bounds are computed from the local Mesh bounds by transforming all eight corners of its AABB. Scene bounds combine those object bounds for visible objects only.

## Applications Above the Layers

Domain-specific applications should depend on this geometry infrastructure rather than being implemented inside it. Robotics, simulation, chess, and other application logic belong above Scene and the kernel; they are not concepts provided by these layers.