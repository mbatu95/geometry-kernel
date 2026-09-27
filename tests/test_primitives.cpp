#include "geometry/intersection.hpp"
#include "geometry/primitives.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>

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

void check_positive_z_normals(const geometry::Mesh& mesh, std::string_view description) {
    for (std::size_t index = 0; index < mesh.triangle_count(); ++index) {
        check(mesh.triangle(index).normal().z > 0.0, description);
    }
}

void test_box() {
    using namespace geometry;

    const Mesh box = make_box(2.0, 4.0, 6.0);
    check(box.vertex_count() == 8, "Box has 8 shared vertices");
    check(box.triangle_count() == 12, "Box has 12 triangles");
    const auto bounds = box.bounding_box();
    check(bounds && bounds->min() == Point3{-1.0, -2.0, -3.0} &&
              bounds->max() == Point3{1.0, 2.0, 3.0},
          "Box has expected centered bounds");
    check(bounds && near(bounds->center(), Point3{0.0, 0.0, 0.0}),
          "Box is centered at origin");

    for (std::size_t index = 0; index < box.triangle_count(); ++index) {
        const Triangle triangle = box.triangle(index);
        const Point3 centroid{
            (triangle.a().x + triangle.b().x + triangle.c().x) / 3.0,
            (triangle.a().y + triangle.b().y + triangle.c().y) / 3.0,
            (triangle.a().z + triangle.b().z + triangle.c().z) / 3.0,
        };
        check(dot(triangle.normal(), centroid - Point3{}) > 0.0,
              "Box triangle normal points outward");
    }

    bool zero_dimension_threw = false;
    try {
        static_cast<void>(make_box(1.0, 0.0, 1.0));
    } catch (const std::invalid_argument&) {
        zero_dimension_threw = true;
    }
    check(zero_dimension_threw, "Box rejects zero dimension");

    bool negative_dimension_threw = false;
    try {
        static_cast<void>(make_box(1.0, -1.0, 1.0));
    } catch (const std::invalid_argument&) {
        negative_dimension_threw = true;
    }
    check(negative_dimension_threw, "Box rejects negative dimension");

    const auto hit = intersect(
        Ray{Point3{0.0, 0.0, 5.0}, Vec3{0.0, 0.0, -1.0}}, box);
    check(hit && near(hit->point, Point3{0.0, 0.0, 3.0}), "Ray intersects generated box");

    const Mesh translated = box.transformed(Transform::translation(0.0, 0.0, 1.0));
    const auto translated_bounds = translated.bounding_box();
    check(translated_bounds && translated_bounds->min() == Point3{-1.0, -2.0, -2.0} &&
              translated_bounds->max() == Point3{1.0, 2.0, 4.0},
          "Translated box has expected bounds");
}

void test_xy_plane() {
    using namespace geometry;

    const Mesh plane = make_xy_plane(6.0, 4.0);
    check(plane.vertex_count() == 4, "XY plane has 4 vertices");
    check(plane.triangle_count() == 2, "XY plane has 2 triangles");
    const auto bounds = plane.bounding_box();
    check(bounds && bounds->min() == Point3{-3.0, -2.0, 0.0} &&
              bounds->max() == Point3{3.0, 2.0, 0.0},
          "XY plane bounds");
    for (const Point3& vertex : plane.vertices()) {
        check(near(vertex.z, 0.0), "XY plane vertices lie at z zero");
    }
    check_positive_z_normals(plane, "XY plane normals point toward +Z");
    const auto hit = intersect(
        Ray{Point3{0.5, 0.5, 2.0}, Vec3{0.0, 0.0, -1.0}}, plane);
    check(hit && near(hit->point, Point3{0.5, 0.5, 0.0}), "Ray from above hits XY plane");

    bool invalid_dimensions_threw = false;
    try {
        static_cast<void>(make_xy_plane(0.0, 2.0));
    } catch (const std::invalid_argument&) {
        invalid_dimensions_threw = true;
    }
    check(invalid_dimensions_threw, "XY plane rejects invalid dimensions");
}

void test_xy_grid() {
    using namespace geometry;

    const Mesh grid = make_xy_grid(8.0, 6.0, 4, 3);
    check(grid.vertex_count() == 20, "XY grid vertex count");
    check(grid.triangle_count() == 24, "XY grid triangle count");
    const auto bounds = grid.bounding_box();
    check(bounds && bounds->min() == Point3{-4.0, -3.0, 0.0} &&
              bounds->max() == Point3{4.0, 3.0, 0.0},
          "XY grid bounds");
    for (const Point3& vertex : grid.vertices()) {
        check(near(vertex.z, 0.0), "XY grid vertices lie at z zero");
    }
    check_positive_z_normals(grid, "XY grid normals point toward +Z");

    const Mesh plane = make_xy_plane(2.0, 2.0);
    const Mesh single_cell = make_xy_grid(2.0, 2.0, 1, 1);
    check(single_cell.vertex_count() == plane.vertex_count() &&
              single_cell.triangle_count() == plane.triangle_count(),
          "1x1 XY grid matches plane topology");
    for (const Point3& grid_vertex : single_cell.vertices()) {
        bool matches_plane_vertex = false;
        for (const Point3& plane_vertex : plane.vertices()) {
            matches_plane_vertex = matches_plane_vertex || near(grid_vertex, plane_vertex);
        }
        check(matches_plane_vertex, "1x1 XY grid matches plane vertices");
    }
    check_positive_z_normals(single_cell, "1x1 XY grid normals point toward +Z");

    bool zero_cells_threw = false;
    try {
        static_cast<void>(make_xy_grid(2.0, 2.0, 0, 1));
    } catch (const std::invalid_argument&) {
        zero_cells_threw = true;
    }
    check(zero_cells_threw, "XY grid rejects zero cell count");

    bool invalid_dimensions_threw = false;
    try {
        static_cast<void>(make_xy_grid(-1.0, 2.0, 1, 1));
    } catch (const std::invalid_argument&) {
        invalid_dimensions_threw = true;
    }
    check(invalid_dimensions_threw, "XY grid rejects invalid dimensions");

    const auto hit = intersect(
        Ray{Point3{1.5, 1.0, 3.0}, Vec3{0.0, 0.0, -1.0}}, grid);
    check(hit && near(hit->point, Point3{1.5, 1.0, 0.0}), "Ray intersects generated XY grid");
}

}  // namespace

int main() {
    test_box();
    test_xy_plane();
    test_xy_grid();
    return failures == 0 ? 0 : 1;
}