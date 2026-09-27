#include "geometry/transform.hpp"

#include <cmath>
#include <iostream>
#include <numbers>
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

void test_identity() {
    using namespace geometry;

    const Transform identity;
    const Point3 point{2.0, -3.0, 4.0};
    const Vec3 vector{-1.0, 5.0, 2.0};

    check(identity.apply_point(point) == point, "default Transform is point identity");
    check(identity.apply_vector(vector) == vector, "default Transform is vector identity");
    check(Transform::identity().matrix() == Mat4::identity(), "Transform identity matrix");
}

void test_translation_and_scaling() {
    using namespace geometry;

    const Transform move = Transform::translation(1.0, 2.0, 3.0);
    const Point3 point = move.apply_point(Point3{4.0, 5.0, 6.0});
    const Vec3 vector = move.apply_vector(Vec3{1.0, 0.0, -1.0});
    check(point == Point3{5.0, 7.0, 9.0}, "translation affects points");
    check(vector == Vec3{1.0, 0.0, -1.0}, "translation does not affect vectors");
    check(Transform::translation(Vec3{1.0, 2.0, 3.0}).matrix() == move.matrix(),
          "translation Vec3 overload");

    check(Transform::scaling(2.0).apply_point(Point3{1.0, -2.0, 3.0}) ==
              Point3{2.0, -4.0, 6.0},
          "uniform scaling");
    check(Transform::scaling(2.0, 3.0, 4.0).apply_point(Point3{1.0, 2.0, 3.0}) ==
              Point3{2.0, 6.0, 12.0},
          "non-uniform scaling");
}

void test_rotations() {
    using namespace geometry;

    constexpr double half_pi = std::numbers::pi / 2.0;
    check(near(Transform::rotation_x(half_pi).apply_vector(Vec3{0.0, 1.0, 0.0}),
               Vec3{0.0, 0.0, 1.0}),
          "rotation around X by pi/2");
    check(near(Transform::rotation_y(half_pi).apply_vector(Vec3{0.0, 0.0, 1.0}),
               Vec3{1.0, 0.0, 0.0}),
          "rotation around Y by pi/2");
    check(near(Transform::rotation_z(half_pi).apply_vector(Vec3{1.0, 0.0, 0.0}),
               Vec3{0.0, 1.0, 0.0}),
          "rotation around Z by pi/2");
}

void test_composition_and_matrix_access() {
    using namespace geometry;

    const Transform translate = Transform::translation(10.0, 0.0, 0.0);
    const Transform scale = Transform::scaling(2.0);
    const Point3 point{1.0, 0.0, 0.0};

    const Transform combined = translate * scale;
    check(combined.apply_point(point) == Point3{12.0, 0.0, 0.0},
          "composition applies right transform first");
    check(combined.apply_point(point) == translate.apply_point(scale.apply_point(point)),
          "composition matches nested applications");
    check((scale * translate).apply_point(point) == Point3{22.0, 0.0, 0.0},
          "composition order is observable");

    const Mat4 expected{1.0, 0.0, 0.0, 10.0,
                        0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 1.0};
    check(translate.matrix() == expected, "translation matrix is consistent");

    bool invalid_matrix_threw = false;
    try {
        static_cast<void>(Transform{Mat4{
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 1.0, 1.0,
        }});
    } catch (const std::invalid_argument&) {
        invalid_matrix_threw = true;
    }
    check(invalid_matrix_threw, "non-affine Mat4 rejected");
}

}  // namespace

int main() {
    test_identity();
    test_translation_and_scaling();
    test_rotations();
    test_composition_and_matrix_access();
    return failures == 0 ? 0 : 1;
}