#include "geometry/intersection.hpp"

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

bool near(const geometry::Vec3& actual, const geometry::Vec3& expected) {
    return near(actual.x, expected.x) && near(actual.y, expected.y) &&
           near(actual.z, expected.z);
}

bool near(const geometry::Point3& actual, const geometry::Point3& expected) {
    return near(actual.x, expected.x) && near(actual.y, expected.y) &&
           near(actual.z, expected.z);
}

void test_triangle() {
    using namespace geometry;

    const Triangle triangle{Point3{0.0, 0.0, 0.0},
                            Point3{2.0, 0.0, 0.0},
                            Point3{0.0, 2.0, 0.0}};
    check(triangle.a() == Point3{0.0, 0.0, 0.0} &&
              triangle.b() == Point3{2.0, 0.0, 0.0} &&
              triangle.c() == Point3{0.0, 2.0, 0.0},
          "Triangle construction and vertex access");
    check(triangle.edge_ab() == Vec3{2.0, 0.0, 0.0}, "Triangle edge AB");
    check(triangle.edge_ac() == Vec3{0.0, 2.0, 0.0}, "Triangle edge AC");
    check(near(triangle.normal(), Vec3{0.0, 0.0, 1.0}), "Triangle normalized normal");
    check(near(triangle.area(), 2.0), "Triangle area");

    bool degenerate_threw = false;
    try {
        static_cast<void>(Triangle{Point3{0.0, 0.0, 0.0},
                                   Point3{1.0, 0.0, 0.0},
                                   Point3{0.0, 1e-14, 0.0}});
    } catch (const std::invalid_argument&) {
        degenerate_threw = true;
    }
    check(degenerate_threw, "Triangle rejects near-zero area");
}

void test_interior_hit() {
    using namespace geometry;

    const Triangle triangle{Point3{0.0, 0.0, 0.0},
                            Point3{2.0, 0.0, 0.0},
                            Point3{0.0, 2.0, 0.0}};
    const auto hit = intersect(Ray{Point3{0.5, 0.5, 1.0}, Vec3{0.0, 0.0, -1.0}}, triangle);

    check(hit.has_value(), "Ray hits triangle interior");
    check(hit && near(hit->t, 1.0), "Interior hit t");
    check(hit && near(hit->point, Point3{0.5, 0.5, 0.0}), "Interior hit point");
    check(hit && near(hit->u, 0.25) && near(hit->v, 0.25),
          "Interior hit barycentric coordinates");

    if (hit) {
        const Point3 barycentric_point = triangle.a() +
            triangle.edge_ab() * hit->u + triangle.edge_ac() * hit->v;
        check(near(hit->point, barycentric_point), "Hit point agrees with barycentric weights");
    }
}

void test_misses_and_boundary_hits() {
    using namespace geometry;

    const Triangle triangle{Point3{0.0, 0.0, 0.0},
                            Point3{2.0, 0.0, 0.0},
                            Point3{0.0, 2.0, 0.0}};
    check(!intersect(Ray{Point3{3.0, 3.0, 1.0}, Vec3{0.0, 0.0, -1.0}}, triangle),
          "Ray misses outside triangle");
    check(!intersect(Ray{Point3{0.5, 0.5, 1.0}, Vec3{1.0, 0.0, 0.0}}, triangle),
          "Ray parallel to triangle plane");
    check(!intersect(Ray{Point3{0.5, 0.5, -1.0}, Vec3{0.0, 0.0, -1.0}}, triangle),
          "Intersection behind ray origin rejected");

    const auto edge_hit = intersect(
        Ray{Point3{1.0, 0.0, 1.0}, Vec3{0.0, 0.0, -1.0}}, triangle);
    check(edge_hit && near(edge_hit->point, Point3{1.0, 0.0, 0.0}), "Edge hit accepted");
    check(edge_hit && near(edge_hit->u, 0.5) && near(edge_hit->v, 0.0),
          "Edge hit barycentric coordinates");

    const auto vertex_hit = intersect(
        Ray{Point3{0.0, 0.0, 1.0}, Vec3{0.0, 0.0, -1.0}}, triangle);
    check(vertex_hit && near(vertex_hit->point, Point3{0.0, 0.0, 0.0}),
          "Vertex hit accepted");
    check(vertex_hit && near(vertex_hit->u, 0.0) && near(vertex_hit->v, 0.0),
          "Vertex hit barycentric coordinates");

    const auto back_hit = intersect(
        Ray{Point3{0.5, 0.5, -1.0}, Vec3{0.0, 0.0, 1.0}}, triangle);
    check(back_hit && near(back_hit->point, Point3{0.5, 0.5, 0.0}),
          "Triangle is hittable from back side");

    const auto non_normalized_hit = intersect(
        Ray{Point3{0.5, 0.5, 1.0}, Vec3{0.0, 0.0, -2.0}}, triangle);
    check(non_normalized_hit && near(non_normalized_hit->t, 0.5),
          "Non-normalized ray direction t");
    check(non_normalized_hit && near(non_normalized_hit->point, Point3{0.5, 0.5, 0.0}),
          "Non-normalized ray direction hit point");
}

}  // namespace

int main() {
    test_triangle();
    test_interior_hit();
    test_misses_and_boundary_hits();
    return failures == 0 ? 0 : 1;
}