# Geometry Conventions

[Back to the project README](../README.md)

This guide records the conventions used by the current implementation. The kernel does not impose a unit system.

## Coordinate System

Points and vectors use Cartesian coordinates. The viewer and XY procedural primitives use the XY plane as the ground plane and +Z as the up direction. The vector cross product and the rotation matrices follow the usual right-handed convention; positive Z rotation maps +X toward +Y.

## Point and Vector

A point represents a location; a vector represents a displacement or direction. The supported 3D operations include:

```text
Point - Point  -> Vector
Point + Vector -> Point
Point - Vector -> Point
```

Adding two points is intentionally not part of the model: locations are not displacements. The same point/vector distinction is provided in 2D.

## Matrices

`Mat3` and `Mat4` store elements in row-major order. This is a storage layout, not a statement about how vectors are multiplied. Transform operations use the column-vector mathematical convention: a point is treated as a column vector and the matrix acts on its left.

The OpenGL viewer explicitly converts matrix elements to OpenGL's expected column-major upload order. The kernel's matrix storage is unchanged by that boundary conversion.

## Transform

`Transform` accepts affine `Mat4` values. Applying a point is equivalent to using homogeneous $w = 1$, so translation affects the result. Applying a vector is equivalent to using homogeneous $w = 0$, so translation does not affect it.

For composition, `A * B` applies `B` first and then `A`, consistent with column-vector multiplication. Transform constructors provide translation, scaling, and axis rotations.

## Ray

A ray is evaluated as:

```text
P(t) = origin + t * direction
```

The direction is not automatically normalized. Therefore `t` is a parameter along the direction and is not necessarily Euclidean distance. A Ray rejects a zero-length direction; intersection routines use forward ray parameters (`t >= 0`).

## Triangle

Triangle orientation follows vertex order: its normal is computed from `(b - a) × (c - a)`. Reversing the winding reverses the normal. The ray-triangle result reports barycentric coordinates with `A = 1 - u - v`, `B = u`, and `C = v`. The ray-triangle query accepts hits from either side; it does not cull back faces.

## AABB

An `AABB` stores minimum and maximum `Point3` corners, with each minimum coordinate no greater than its corresponding maximum. Point containment includes the boundary.

Ray intersection uses the slab method. A zero direction component is handled as a parallel slab: the ray misses if its origin lies outside that slab. Boundary and grazing contacts are accepted, and a scale-aware interval tolerance is used for numerical comparisons. The reported `t_enter` is clamped to zero when the ray starts inside; `t_exit` remains the exit parameter.

## Mesh

`Mesh` stores a vertex array and indexed triangle triplets rather than duplicated Triangle objects. Construction validates all indices and rejects degenerate indexed triangles using the same behavior as `Triangle`. An empty Mesh is valid and has no bounding box. `AABB::from_triangle` constructs an AABB from a Triangle; the procedural generators return ordinary indexed Mesh values.