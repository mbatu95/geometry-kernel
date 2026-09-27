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

}  // namespace

int main() {
    test_construction_and_access();
    test_empty_mesh_and_bounds();
    test_transformed_mesh();
    test_ray_mesh_intersections();
    return failures == 0 ? 0 : 1;
}