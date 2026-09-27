#include "geometry/intersection.hpp"

#include <cmath>
#include <iostream>
#include <limits>
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

bool near(const geometry::Vec3& actual, const geometry::Vec3& expected) {
    return near(actual.x, expected.x) && near(actual.y, expected.y) &&
           near(actual.z, expected.z);
}

void test_aabb_value_type() {
    using namespace geometry;

    AABB box{Point3{-1.0, -2.0, -3.0}, Point3{3.0, 2.0, 1.0}};
    check(box.min() == Point3{-1.0, -2.0, -3.0}, "AABB minimum accessor");
    check(box.max() == Point3{3.0, 2.0, 1.0}, "AABB maximum accessor");
    check(near(box.center(), Point3{1.0, 0.0, -1.0}), "AABB center");
    check(near(box.size(), Vec3{4.0, 4.0, 4.0}), "AABB size");
    check(box.contains(Point3{0.0, 0.0, 0.0}), "AABB contains interior point");
    check(box.contains(Point3{3.0, 2.0, -3.0}), "AABB contains boundary point");
    check(!box.contains(Point3{3.1, 0.0, 0.0}), "AABB rejects outside point");

    bool invalid_bounds_threw = false;
    try {
        static_cast<void>(AABB{Point3{1.0, 0.0, 0.0}, Point3{0.0, 1.0, 1.0}});
    } catch (const std::invalid_argument&) {
        invalid_bounds_threw = true;
    }
    check(invalid_bounds_threw, "AABB rejects invalid bounds");

    bool nan_bounds_threw = false;
    try {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        static_cast<void>(AABB{Point3{nan, 0.0, 0.0}, Point3{1.0, 1.0, 1.0}});
    } catch (const std::invalid_argument&) {
        nan_bounds_threw = true;
    }
    check(nan_bounds_threw, "AABB rejects unordered NaN bounds");

    box.expand(Point3{-2.0, 4.0, 2.0});
    check(box.min() == Point3{-2.0, -2.0, -3.0}, "AABB expand updates minimum");
    check(box.max() == Point3{3.0, 4.0, 2.0}, "AABB expand updates maximum");
    check(box.contains(Point3{-2.0, 4.0, 2.0}), "Expanded AABB contains supplied point");

    const Triangle triangle{Point3{-2.0, 1.0, 3.0}, Point3{4.0, -1.0, 0.0},
                            Point3{1.0, 5.0, -2.0}};
    const AABB triangle_box = AABB::from_triangle(triangle);
    check(triangle_box.min() == Point3{-2.0, -1.0, -2.0}, "AABB from Triangle minimum");
    check(triangle_box.max() == Point3{4.0, 5.0, 3.0}, "AABB from Triangle maximum");
}

void test_ray_intersections() {
    using namespace geometry;

    const AABB box{Point3{0.0, 0.0, 0.0}, Point3{1.0, 1.0, 1.0}};
    const auto through = intersect(
        Ray{Point3{-1.0, 0.5, 0.5}, Vec3{1.0, 0.0, 0.0}}, box);
    check(through.has_value(), "Ray passes through AABB");
    check(through && near(through->t_enter, 1.0) && near(through->t_exit, 2.0),
          "AABB through-ray t values");
    check(through && near(through->enter_point, Point3{0.0, 0.5, 0.5}) &&
              near(through->exit_point, Point3{1.0, 0.5, 0.5}),
          "AABB through-ray points");

    check(!intersect(Ray{Point3{-1.0, 2.0, 0.5}, Vec3{1.0, 0.0, 0.0}}, box),
          "Ray misses AABB");

    const auto inside = intersect(
        Ray{Point3{0.5, 0.5, 0.5}, Vec3{2.0, 0.0, 0.0}}, box);
    check(inside && near(inside->t_enter, 0.0) && near(inside->t_exit, 0.25),
          "Ray starting inside uses zero entry parameter");
    check(inside && near(inside->enter_point, Point3{0.5, 0.5, 0.5}) &&
              near(inside->exit_point, Point3{1.0, 0.5, 0.5}),
          "Ray starting inside entry and exit points");

    const auto parallel_inside_slabs = intersect(
        Ray{Point3{0.5, -1.0, 0.5}, Vec3{0.0, 1.0, 0.0}}, box);
    check(parallel_inside_slabs && near(parallel_inside_slabs->t_enter, 1.0) &&
              near(parallel_inside_slabs->t_exit, 2.0),
          "Ray parallel to two slabs and within them");
    check(!intersect(Ray{Point3{2.0, -1.0, 0.5}, Vec3{0.0, 1.0, 0.0}}, box),
          "Ray parallel to slab and outside AABB");

    const auto grazing = intersect(
        Ray{Point3{-1.0, 1.0, 0.5}, Vec3{1.0, 0.0, 0.0}}, box);
    check(grazing && near(grazing->t_enter, 1.0) && near(grazing->t_exit, 2.0),
          "Boundary grazing hit accepted");

    const auto non_normalized = intersect(
        Ray{Point3{-2.0, 0.5, 0.5}, Vec3{2.0, 0.0, 0.0}}, box);
    check(non_normalized && near(non_normalized->t_enter, 1.0) &&
              near(non_normalized->t_exit, 1.5),
          "Non-normalized ray direction parameters");

    check(!intersect(Ray{Point3{2.0, 0.5, 0.5}, Vec3{1.0, 0.0, 0.0}}, box),
          "Intersection entirely behind ray is rejected");
}

}  // namespace

int main() {
    test_aabb_value_type();
    test_ray_intersections();
    return failures == 0 ? 0 : 1;
}