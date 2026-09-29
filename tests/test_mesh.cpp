#include "geometry/intersection.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, std::string_view description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

bool near(double actual, double expected) {
    return std::abs(actual - expected) <= 1e-9;
}

bool near(const geometry::Point3& actual, const geometry::Point3& expected) {
    return near(actual.x, expected.x) && near(actual.y, expected.y) &&
           near(actual.z, expected.z);
}

geometry::Mesh make_single_triangle() {
    using namespace geometry;
    return Mesh{
        {Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0}},
        {TriangleIndices{0, 1, 2}},
    };
}

void test_construction_and_access() {
    using namespace geometry;

    const Mesh mesh{
        {Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0}},
        {TriangleIndices{0, 1, 2}},
    };
    check(mesh.vertex_count() == 3, "Mesh vertex count");
    check(mesh.triangle_count() == 1, "Mesh triangle count");
    check(mesh.vertices().size() == 3, "Mesh read-only vertex access");
    check(mesh.triangle_indices().size() == 1 &&
              mesh.triangle_indices()[0] == TriangleIndices{0, 1, 2},
          "Mesh read-only triangle-index access");

    const Mesh small_mesh{
        {Point3{0.0, 0.0, 0.0}, Point3{1e-8, 0.0, 0.0}, Point3{0.0, 1e-8, 0.0}},
        {TriangleIndices{0, 1, 2}},
    };
    check(small_mesh.triangle_count() == 1 && small_mesh.triangle(0).area() > 0.0,
          "Mesh accepts a valid small triangle");

    const Triangle triangle = mesh.triangle(0);
    check(triangle.a() == Point3{0.0, 0.0, 0.0} &&
              triangle.b() == Point3{1.0, 0.0, 0.0} &&
              triangle.c() == Point3{0.0, 1.0, 0.0},
          "Mesh triangle retrieval");

    bool invalid_vertex_threw = false;
    try {
        static_cast<void>(Mesh{
            {Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}},
            {TriangleIndices{0, 1, 2}},
        });
    } catch (const std::invalid_argument&) {
        invalid_vertex_threw = true;
    }
    check(invalid_vertex_threw, "Mesh rejects out-of-range vertex index");

    bool degenerate_threw = false;
    try {
        static_cast<void>(Mesh{
            {Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{2.0, 0.0, 0.0}},
            {TriangleIndices{0, 1, 2}},
        });
    } catch (const std::invalid_argument&) {
        degenerate_threw = true;
    }
    check(degenerate_threw, "Mesh rejects degenerate indexed triangle");

    bool invalid_triangle_threw = false;
    try {
        static_cast<void>(mesh.triangle(1));
    } catch (const std::out_of_range&) {
        invalid_triangle_threw = true;
    }
    check(invalid_triangle_threw, "Mesh rejects invalid triangle access");
}

void test_empty_mesh_and_bounds() {
    using namespace geometry;

    const Mesh empty;
    check(empty.vertex_count() == 0 && empty.triangle_count() == 0, "Empty Mesh counts");
    check(!empty.bounding_box(), "Empty Mesh has no bounding box");
    check(!intersect(Ray{Point3{0.0, 0.0, 1.0}, Vec3{0.0, 0.0, -1.0}}, empty),
          "Empty Mesh has no ray intersection");

    const Mesh mesh{
        {Point3{-1.0, 2.0, 3.0}, Point3{4.0, -2.0, 1.0}, Point3{0.0, 1.0, 5.0}},
        {TriangleIndices{0, 1, 2}},
    };
    const std::optional<AABB> bounds = mesh.bounding_box();
    check(bounds.has_value(), "Non-empty Mesh has a bounding box");
    check(bounds && bounds->min() == Point3{-1.0, -2.0, 1.0} &&
              bounds->max() == Point3{4.0, 2.0, 5.0},
          "Mesh bounding box encloses all vertices");
}

void test_transformed_mesh() {
    using namespace geometry;

    const Mesh original = make_single_triangle();
    const Mesh moved = original.transformed(Transform::translation(3.0, -2.0, 4.0));
    check(moved.vertices()[0] == Point3{3.0, -2.0, 4.0}, "Transformed Mesh moves vertices");
    check(moved.triangle_indices() == original.triangle_indices(),
          "Transformed Mesh preserves connectivity");
    check(original.vertices()[0] == Point3{0.0, 0.0, 0.0},
          "Transform leaves original Mesh unchanged");
    check(near(moved.triangle(0).a(), Point3{3.0, -2.0, 4.0}),
          "Transformed Mesh triangle uses transformed vertices");
}

void test_ray_mesh_intersections() {
    using namespace geometry;

    const Mesh single = make_single_triangle();
    const auto single_hit = intersect(
        Ray{Point3{0.25, 0.5, 3.0}, Vec3{0.0, 0.0, -2.0}}, single);
    check(single_hit.has_value(), "Ray hits single-triangle Mesh");
    check(single_hit && near(single_hit->t, 1.5), "Mesh hit retains non-normalized ray t");
    check(single_hit && single_hit->triangle_index == 0, "Single-triangle Mesh index");
    check(single_hit && near(single_hit->point, Point3{0.25, 0.5, 0.0}), "Mesh hit point");
    check(single_hit && near(single_hit->u, 0.25) && near(single_hit->v, 0.5),
          "Mesh hit barycentric coordinates");

    check(!intersect(Ray{Point3{2.0, 2.0, 1.0}, Vec3{0.0, 0.0, -1.0}}, single),
          "Ray misses Mesh");
    check(!intersect(Ray{Point3{0.25, 0.5, -1.0}, Vec3{0.0, 0.0, -1.0}}, single),
          "Ray entirely behind Mesh is rejected");

    const Mesh layered{
        {
            Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0},
            Point3{0.0, 0.0, 2.0}, Point3{1.0, 0.0, 2.0}, Point3{0.0, 1.0, 2.0},
        },
        {TriangleIndices{0, 1, 2}, TriangleIndices{3, 4, 5}},
    };
    const auto closest = intersect(
        Ray{Point3{0.25, 0.5, 3.0}, Vec3{0.0, 0.0, -1.0}}, layered);
    check(closest.has_value(), "Ray hits multi-triangle Mesh");
    check(closest && closest->triangle_index == 1, "Closest Mesh triangle is returned");
    check(closest && near(closest->t, 1.0) &&
              near(closest->point, Point3{0.25, 0.5, 2.0}),
          "Closest Mesh hit details");
}

geometry::Mesh make_tetrahedron() {
    using namespace geometry;
    // A closed, watertight tetrahedron with outward-facing windings.
    return Mesh{
        {
            Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0},
            Point3{0.0, 1.0, 0.0}, Point3{0.0, 0.0, 1.0},
        },
        {
            TriangleIndices{0, 2, 1},
            TriangleIndices{0, 1, 3},
            TriangleIndices{0, 3, 2},
            TriangleIndices{1, 2, 3},
        },
    };
}

geometry::Mesh make_unit_cube() {
    using namespace geometry;
    // A closed, watertight unit cube spanning [0,1]^3 with outward-facing windings.
    const std::vector<Point3> vertices{
        Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{1.0, 1.0, 0.0},
        Point3{0.0, 1.0, 0.0}, Point3{0.0, 0.0, 1.0}, Point3{1.0, 0.0, 1.0},
        Point3{1.0, 1.0, 1.0}, Point3{0.0, 1.0, 1.0},
    };
    const std::vector<TriangleIndices> triangles{
        // -z face
        TriangleIndices{0, 2, 1}, TriangleIndices{0, 3, 2},
        // +z face
        TriangleIndices{4, 5, 6}, TriangleIndices{4, 6, 7},
        // -y face
        TriangleIndices{0, 1, 5}, TriangleIndices{0, 5, 4},
        // +y face
        TriangleIndices{3, 7, 6}, TriangleIndices{3, 6, 2},
        // -x face
        TriangleIndices{0, 4, 7}, TriangleIndices{0, 7, 3},
        // +x face
        TriangleIndices{1, 2, 6}, TriangleIndices{1, 6, 5},
    };
    return Mesh{vertices, triangles};
}

void test_compute_aabb_free_function() {
    using namespace geometry;

    const std::vector<Point3> empty_vertices;
    check(!compute_aabb(empty_vertices), "compute_aabb on empty vertex list is nullopt");

    const std::vector<Point3> vertices{
        Point3{-1.0, 2.0, 3.0}, Point3{4.0, -2.0, 1.0}, Point3{0.0, 1.0, 5.0}};
    const std::optional<AABB> bounds = compute_aabb(vertices);
    check(bounds.has_value() && bounds->min() == Point3{-1.0, -2.0, 1.0} &&
              bounds->max() == Point3{4.0, 2.0, 5.0},
          "compute_aabb(vertices) matches Mesh::bounding_box()");

    const Mesh mesh{vertices, {TriangleIndices{0, 1, 2}}};
    const std::optional<AABB> mesh_bounds = compute_aabb(mesh);
    check(mesh_bounds.has_value() && mesh.bounding_box().has_value() &&
              mesh_bounds->min() == mesh.bounding_box()->min() &&
              mesh_bounds->max() == mesh.bounding_box()->max(),
          "compute_aabb(mesh) matches member function");

    const Mesh cube = make_unit_cube();
    const std::optional<AABB> cube_bounds = compute_aabb(cube);
    check(cube_bounds.has_value() && cube_bounds->min() == Point3{0.0, 0.0, 0.0} &&
              cube_bounds->max() == Point3{1.0, 1.0, 1.0},
          "compute_aabb on a unit cube matches its known extent");
}

void test_compute_face_normals_free_function() {
    using namespace geometry;

    const Mesh mesh = make_single_triangle();
    const std::vector<Vec3> normals = compute_face_normals(mesh.vertices(), mesh.triangle_indices());
    check(normals.size() == 1, "compute_face_normals returns one normal per triangle");
    check(near(normals[0].x, 0.0) && near(normals[0].y, 0.0) && near(normals[0].z, 1.0),
          "compute_face_normals gives the correct outward normal");

    const std::vector<Vec3> mesh_normals = compute_face_normals(mesh);
    check(mesh_normals.size() == 1 && near(mesh_normals[0].x, normals[0].x) &&
              near(mesh_normals[0].y, normals[0].y) && near(mesh_normals[0].z, normals[0].z),
          "compute_face_normals(mesh) matches vertex/index overload");

    // Out-of-range indices produce a zero normal rather than throwing.
    const std::vector<Point3> vertices{Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}};
    const std::vector<TriangleIndices> bad_indices{TriangleIndices{0, 1, 5}};
    const std::vector<Vec3> bad_normals = compute_face_normals(vertices, bad_indices);
    check(bad_normals.size() == 1 && bad_normals[0].x == 0.0 && bad_normals[0].y == 0.0 &&
              bad_normals[0].z == 0.0,
          "compute_face_normals gives a zero normal for out-of-range indices");

    // Zero-area (collinear) triangle also produces a zero normal.
    const std::vector<Point3> collinear{
        Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{2.0, 0.0, 0.0}};
    const std::vector<Vec3> degenerate_normals =
        compute_face_normals(collinear, {TriangleIndices{0, 1, 2}});
    check(degenerate_normals.size() == 1 && degenerate_normals[0].x == 0.0 &&
              degenerate_normals[0].y == 0.0 && degenerate_normals[0].z == 0.0,
          "compute_face_normals gives a zero normal for a degenerate triangle");

    // All normalized normals on a unit cube should have unit length and point
    // outward along a single axis.
    const Mesh cube = make_unit_cube();
    const std::vector<Vec3> cube_normals = compute_face_normals(cube);
    check(cube_normals.size() == cube.triangle_count(),
          "compute_face_normals returns one normal per cube triangle");
    bool all_unit_length = true;
    for (const Vec3& normal : cube_normals) {
        if (!near(normal.length(), 1.0)) {
            all_unit_length = false;
        }
    }
    check(all_unit_length, "compute_face_normals on a unit cube are normalized");
    check(near(cube_normals[0].z, -1.0), "compute_face_normals -z cube face points down");
    check(near(cube_normals[2].z, 1.0), "compute_face_normals +z cube face points up");
}

void test_has_degenerate_triangles_free_function() {
    using namespace geometry;

    const std::vector<Point3> vertices{
        Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0}};
    check(!has_degenerate_triangles(vertices, {TriangleIndices{0, 1, 2}}),
          "A valid triangle is not degenerate");
    check(has_degenerate_triangles(vertices, {TriangleIndices{0, 1, 5}}),
          "An out-of-range index is reported as degenerate");

    const std::vector<Point3> collinear{
        Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{2.0, 0.0, 0.0}};
    check(has_degenerate_triangles(collinear, {TriangleIndices{0, 1, 2}}),
          "A zero-area (collinear) triangle is reported as degenerate");

    check(!has_degenerate_triangles(std::vector<Point3>{}, std::vector<TriangleIndices>{}),
          "An empty triangle list has no degenerate triangles");

    const Mesh mesh = make_single_triangle();
    check(!has_degenerate_triangles(mesh), "has_degenerate_triangles(mesh) matches free function");
}

void test_is_watertight_free_function() {
    using namespace geometry;

    check(is_watertight(std::vector<TriangleIndices>{}),
          "An empty triangle list is vacuously watertight");

    const Mesh open_triangle = make_single_triangle();
    check(!is_watertight(open_triangle), "A single open triangle is not watertight");

    const Mesh tetrahedron = make_tetrahedron();
    check(is_watertight(tetrahedron), "A closed tetrahedron is watertight");

    const Mesh cube = make_unit_cube();
    check(is_watertight(cube), "A closed unit cube is watertight");

    // Adding a third triangle on the shared edge {0,1} makes it non-manifold
    // (shared by three triangles instead of two), so the mesh is not watertight.
    const Mesh non_manifold{
        tetrahedron.vertices(),
        {
            TriangleIndices{0, 2, 1},
            TriangleIndices{0, 1, 3},
            TriangleIndices{0, 3, 2},
            TriangleIndices{1, 2, 3},
            TriangleIndices{0, 1, 2},
        },
    };
    check(!is_watertight(non_manifold), "A non-manifold edge (shared by 3 triangles) fails watertight");
}

}  // namespace

int main() {
    test_construction_and_access();
    test_empty_mesh_and_bounds();
    test_transformed_mesh();
    test_ray_mesh_intersections();
    test_compute_aabb_free_function();
    test_compute_face_normals_free_function();
    test_has_degenerate_triangles_free_function();
    test_is_watertight_free_function();
    return failures == 0 ? 0 : 1;
}