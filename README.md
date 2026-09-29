# Geometry Kernel

A lightweight modern C++20 geometry foundation providing mathematical primitives, geometric queries, indexed triangle meshes, scene representation, and an optional OpenGL viewer.

The project is early-stage and is being prepared for its first `v0.1.0` release.

## Features

### Math

- `Vec2` / `Vec3` and `Point2` / `Point3`, using `double` coordinates
- `Mat3` / `Mat4` with row-major storage
- Affine `Transform` with separate point and vector operations

### Geometry

- `Ray`, `Plane`, and `Triangle`
- Axis-aligned bounding box (`AABB`)
- Indexed triangle `Mesh`

### Mesh Foundation

- `TriangleIndex` (alias of `TriangleIndices`): a triangle as three vertex indices `a`, `b`, `c`
- `Mesh`: an indexed triangle mesh over `std::vector<Point3>` vertices and `std::vector<TriangleIndices>` triangles; empty meshes are valid and their `bounding_box()` is `std::nullopt`
- `compute_aabb()`: axis-aligned bounding box over raw vertices or a `Mesh`
- `compute_face_normals()`: per-triangle unit normals from raw vertices/indices or a `Mesh`; degenerate or out-of-range triangles yield a zero normal rather than throwing
- `has_degenerate_triangles()`: detects triangles with out-of-range vertex indices or zero area
- `is_watertight()`: checks that every undirected edge is shared by exactly two triangles

### Intersection Queries

- Ray / Plane
- Ray / Triangle
- Ray / AABB
- Ray / Mesh

### Procedural Geometry

- Box
- XY Plane
- XY Grid

### Scene

- Named `SceneObject` values containing a local Mesh and world Transform
- Object visibility, lookup, and removal
- Transformed object world-space bounds
- Scene bounds over visible objects

### Viewer

- Optional OpenGL viewer using GLFW
- Perspective orbit camera and mouse-wheel zoom
- Indexed Mesh rendering
- Window resizing and depth testing

## Architecture

Dependencies point from applications toward the reusable lower layers:

```text
Application
	│
	▼
geometry_scene
	│
	▼
geometry_kernel

geometry_viewer
	│
	▼
geometry_scene
	│
	▼
geometry_kernel
```

`geometry_kernel` provides mathematical and geometric functionality. `geometry_scene` organizes named objects, transforms, and visibility. The optional `geometry_viewer` provides OpenGL visualization and interaction. The core geometry kernel has no dependency on OpenGL or GLFW.

See [Architecture](docs/ARCHITECTURE.md), [Geometry Conventions](docs/GEOMETRY.md), and [Viewer](docs/VIEWER.md) for more detail.

## Design Principles

- C++20 and minimal external dependencies
- Double-precision geometry, independent of rendering
- Unit-agnostic coordinates; applications are responsible for choosing consistent units
- Explicit Point versus Vector semantics
- Indexed triangle meshes and simple value-oriented types
- Row-major matrix storage with a column-vector mathematical convention
- Affine transforms distinguish points from vectors
- Ray directions need not be normalized
- Numerical tolerances are algorithm-specific; there is no universal global epsilon
- No polymorphic geometry hierarchy

## Repository Structure

```text
include/geometry/   Header-only mathematical and geometry types
include/scene/      SceneObject and Scene headers
viewer/             Optional GLFW/OpenGL application and shaders
tests/              Geometry kernel and Scene tests
examples/           Small standalone kernel examples
docs/               Architecture, geometry, and viewer documentation
src/                Placeholder; current library implementations are headers
CMakeLists.txt      Targets, options, and test registration
```

## Requirements

The geometry kernel and Scene tests require a C++20 compiler and CMake 3.20 or newer.

Building the optional viewer additionally requires OpenGL and GLFW 3.3 or newer. On macOS, install the build tools and GLFW with Homebrew:

```sh
brew install cmake glfw
```

The viewer requests an OpenGL 3.3 Core context. Other platforms need compatible OpenGL and GLFW development packages discoverable by CMake.
The viewer defaults to enabled for a top-level checkout and disabled when this project is included as a CMake subproject. `GEOMETRY_KERNEL_BUILD_VIEWER` can explicitly override either default.

## Build

```sh
git clone https://github.com/mbatu95/geometry-kernel.git
cd geometry-kernel

cmake -S . -B build
cmake --build build
```

## Kernel-Only Build

To build the geometry kernel, examples, and tests without searching for viewer dependencies:

```sh
cmake -S . -B build \
	-DGEOMETRY_KERNEL_BUILD_VIEWER=OFF

cmake --build build
```

## Running Tests

```sh
ctest --test-dir build --output-on-failure
```

## Running the Viewer

From the repository root, run:

```sh
./build/geometry_viewer
```

The current demo displays a procedural XY grid and a box. Use left mouse drag to orbit, the mouse wheel to zoom, and Escape to close the viewer.

## Usage Example

```cpp
#include <geometry/primitives.hpp>
#include <geometry/transform.hpp>

auto box = geometry::make_box(1.0, 1.0, 1.0);
auto moved = box.transformed(
	geometry::Transform::translation(0.0, 0.0, 2.0));
```

## Mesh Foundation Usage Example

```cpp
#include <geometry/mesh.hpp>

using geometry::Mesh;
using geometry::Point3;
using geometry::TriangleIndex;

std::vector<Point3> vertices{
	Point3{0.0, 0.0, 0.0},
	Point3{1.0, 0.0, 0.0},
	Point3{0.0, 1.0, 0.0},
};
std::vector<TriangleIndex> triangles{TriangleIndex{0, 1, 2}};

Mesh mesh{vertices, triangles};

auto bounds = geometry::compute_aabb(mesh);
auto normals = geometry::compute_face_normals(mesh);
bool degenerate = geometry::has_degenerate_triangles(mesh);
bool watertight = geometry::is_watertight(mesh);
```

## Roadmap

Possible future areas include additional procedural primitives, mesh attributes such as normals and UVs, spatial acceleration structures, more geometric queries, import/export, and improved viewer tooling. These are not currently implemented.

## Status

The project is experimental and under active development. APIs may change before a stable release.