# Repository Survey: Structure and Conventions (Mesh Integration)

[Back to the project README](../README.md)

This note records the findings from surveying the existing `geometry-kernel`
layout ahead of further mesh-related work, and the conventions any new mesh
code must follow to integrate without breaking public APIs.

## Layout

- `include/geometry/*.hpp` — header-only math/geometry kernel (`Vec2/3`,
  `Point2/3`, `Mat3/4`, `Transform`, `Ray`, `Plane`, `Triangle`, `AABB`,
  `Mesh`, `primitives.hpp` procedural generators, `intersection.hpp` queries).
- `include/scene/*.hpp` — thin organization layer (`SceneObject`, `Scene`)
  depending on the kernel only.
- `viewer/` — optional OpenGL/GLFW application depending on kernel + scene.
- `examples/*.cpp` — one small runnable example per feature area.
- `tests/*.cpp` — one test file per feature area, each wired to its own
  CTest executable in `CMakeLists.txt`.
- `docs/ARCHITECTURE.md` and `docs/GEOMETRY.md` — design/API documentation.

## Build/Test Tooling

- CMake ≥ 3.20, C++20, header-only `geometry_kernel` INTERFACE library
  (`geometry::geometry_kernel` alias) plus a `geometry_scene` INTERFACE
  library (`scene::scene` alias) that links against the kernel.
- Every test/example file gets its own `add_executable` + `target_link_libraries`
  + (for tests) `add_test` block in the root `CMakeLists.txt`; there is no
  test framework, just plain `assert`-style checks returning from `main`.
- `-Wall -Wextra -Wpedantic` enabled for GNU/Clang/AppleClang.
- Viewer build is optional via `GEOMETRY_KERNEL_BUILD_VIEWER` and only
  pulled in for top-level checkouts.

## Existing Mesh Support

Mesh-related code already exists and follows the established conventions:

- `include/geometry/mesh.hpp`: `TriangleIndices` (three `std::size_t`) and
  `Mesh` (vector of `Point3` vertices + vector of `TriangleIndices`),
  validating indices in the constructor and exposing `triangle(index)`,
  `vertex_count()`, `triangle_count()`, and a cached `bounding_box()`.
- `include/geometry/primitives.hpp`: procedural mesh generators (box, XY
  plane, XY grid) returning `Mesh` values.
- `include/geometry/intersection.hpp`: Ray/Mesh query built on top of the
  existing Ray/Triangle and Ray/AABB queries.
- `include/scene/scene_object.hpp` / `scene.hpp`: `SceneObject` holds a
  local-space `Mesh` plus a world `Transform`; `Scene` aggregates objects
  and computes bounds by transforming mesh AABB corners.
- Tests: `tests/test_mesh.cpp`, `tests/test_primitives.cpp`,
  `tests/test_scene.cpp`; example: `examples/mesh_example.cpp`.

## Conventions for New Mesh-Related Code

1. Keep new geometry types header-only under `include/geometry/`, using
   `double` coordinates and the existing `Point3`/`Vec3`/`Mat3`/`Mat4`
   types rather than introducing parallel math types.
2. Do not add graphics (OpenGL/GLFW) dependencies to `geometry_kernel` or
   `geometry_scene`; rendering-specific conversions stay in `viewer/`.
3. Preserve existing public API shapes (`Mesh`, `TriangleIndices`,
   `SceneObject`, `Scene`) — extend via new free functions/queries or
   additive members rather than changing existing signatures.
4. Add a matching test file under `tests/` and register it with its own
   `add_executable`/`add_test` pair in `CMakeLists.txt`, mirroring the
   pattern used for `test_mesh.cpp`/`test_primitives.cpp`.
5. Add a short runnable example under `examples/` when introducing a new
   user-facing feature, matching `mesh_example.cpp`.
6. Update `docs/ARCHITECTURE.md`/`docs/GEOMETRY.md` when introducing new
   layers or changing the dependency direction; new mesh utilities that
   only depend on the kernel belong in `geometry_kernel`, not `geometry_scene`.

## Conclusion

No structural gaps were found: the kernel already has an indexed `Mesh`
type, procedural generators, and Ray/Mesh intersection, all following a
consistent header-only, dependency-light pattern. Further mesh-related
work (e.g., new mesh algorithms or queries) should be added under
`include/geometry/`, tested under `tests/`, and wired into
`CMakeLists.txt` using the same per-feature executable pattern, without
touching the `geometry_scene`/`geometry_viewer` layering.
