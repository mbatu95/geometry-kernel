#include "geometry/intersection.hpp"

#include <cmath>
#include <iostream>
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

void test_ray() {
    using namespace geometry;

    const Ray ray{Point3{1.0, 2.0, 3.0}, Vec3{0.0, 0.0, 2.0}};
    check(ray.origin() == Point3{1.0, 2.0, 3.0}, "Ray origin accessor");
    check(ray.direction() == Vec3{0.0, 0.0, 2.0}, "Ray direction accessor");
    check(ray.point_at(0.0) == Point3{1.0, 2.0, 3.0}, "Ray point_at zero");
    check(ray.point_at(2.0) == Point3{1.0, 2.0, 7.0}, "Ray point_at positive t");
    check(ray.point_at(1.0) == Point3{1.0, 2.0, 5.0},
          "Ray preserves non-normalized direction");

    bool zero_direction_threw = false;
    try {
        static_cast<void>(Ray{Point3{}, Vec3{}});
    } catch (const std::invalid_argument&) {
        zero_direction_threw = true;
    }
    check(zero_direction_threw, "Ray rejects zero direction");
}

void test_plane() {
    using namespace geometry;

    const Plane plane{Point3{0.0, 0.0, 2.0}, Vec3{0.0, 0.0, 4.0}};
    check(plane.point() == Point3{0.0, 0.0, 2.0}, "Plane point accessor");
    check(near(plane.normal().length(), 1.0), "Plane normal is normalized");
    check(plane.normal() == Vec3{0.0, 0.0, 1.0}, "Plane normalized normal accessor");
    check(near(plane.signed_distance(Point3{0.0, 0.0, 5.0}), 3.0),
          "Plane signed distance on positive side");
    check(near(plane.signed_distance(Point3{0.0, 0.0, -1.0}), -3.0),
          "Plane signed distance on negative side");
    check(near(plane.signed_distance(Point3{5.0, -3.0, 2.0}), 0.0),
          "Plane signed distance on plane");

    bool zero_normal_threw = false;
    try {
        static_cast<void>(Plane{Point3{}, Vec3{}});
    } catch (const std::invalid_argument&) {
        zero_normal_threw = true;
    }
    check(zero_normal_threw, "Plane rejects zero normal");
}

void test_intersection() {
    using namespace geometry;

    const Plane plane{Point3{0.0, 0.0, 0.0}, Vec3{0.0, 0.0, 1.0}};
    const auto hit = intersect(
        Ray{Point3{1.0, 2.0, 5.0}, Vec3{0.0, 0.0, -1.0}}, plane);
    check(hit.has_value(), "Ray hits plane");
    check(hit && near(hit->t, 5.0), "Ray-plane intersection t");
    check(hit && near(hit->point, Point3{1.0, 2.0, 0.0}),
          "Ray-plane intersection point");

    check(!intersect(Ray{Point3{0.0, 0.0, 1.0}, Vec3{1.0, 0.0, 0.0}}, plane),
          "Parallel ray does not intersect plane");
    check(!intersect(Ray{Point3{0.0, 0.0, -1.0}, Vec3{0.0, 0.0, -1.0}}, plane),
          "Intersection behind ray origin is rejected");

    const auto starts_on_plane = intersect(
        Ray{Point3{2.0, 3.0, 0.0}, Vec3{0.0, 0.0, 1.0}}, plane);
    check(starts_on_plane && near(starts_on_plane->t, 0.0),
          "Ray beginning on plane hits at t zero");
    check(starts_on_plane && near(starts_on_plane->point, Point3{2.0, 3.0, 0.0}),
          "Ray beginning on plane intersection point");

    const auto non_normalized_hit = intersect(
        Ray{Point3{0.0, 0.0, 5.0}, Vec3{0.0, 0.0, -2.0}}, plane);
    check(non_normalized_hit && near(non_normalized_hit->t, 2.5),
          "Non-normalized direction intersection t");
    check(non_normalized_hit && near(non_normalized_hit->point, Point3{0.0, 0.0, 0.0}),
          "Non-normalized direction intersection point");
}

}  // namespace

int main() {
    test_ray();
    test_plane();
    test_intersection();
    return failures == 0 ? 0 : 1;
}