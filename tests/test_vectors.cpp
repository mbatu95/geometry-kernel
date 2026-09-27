#include "geometry/vec2.hpp"
#include "geometry/vec3.hpp"

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

void test_vec2() {
    using geometry::Vec2;

    const Vec2 zero;
    const Vec2 first{3.0, 4.0};
    const Vec2 second{1.0, 2.0};

    check(zero == Vec2{0.0, 0.0}, "Vec2 default construction");
    check(first.x == 3.0 && first.y == 4.0, "Vec2 value construction");
    check(first + second == Vec2{4.0, 6.0}, "Vec2 addition");
    check(first - second == Vec2{2.0, 2.0}, "Vec2 subtraction");
    check(-first == Vec2{-3.0, -4.0}, "Vec2 unary negation");
    check(first * 2.0 == Vec2{6.0, 8.0}, "Vec2 right scalar multiplication");
    check(2.0 * first == Vec2{6.0, 8.0}, "Vec2 left scalar multiplication");
    check(first / 2.0 == Vec2{1.5, 2.0}, "Vec2 division");
    check(geometry::dot(first, second) == 11.0, "Vec2 dot product");
    check(first.length_squared() == 25.0, "Vec2 squared length");
    check(near(first.length(), 5.0), "Vec2 length");
    check(near(first.normalized().x, 0.6) && near(first.normalized().y, 0.8),
          "Vec2 normalization");
    check(zero.normalized() == zero, "Vec2 zero normalization");
    check(first != second, "Vec2 inequality");

    Vec2 compound{1.0, 2.0};
    compound += second;
    compound -= Vec2{1.0, 1.0};
    compound *= 2.0;
    compound /= 2.0;
    check(compound == Vec2{1.0, 3.0}, "Vec2 compound operators");

    bool division_threw = false;
    try {
        static_cast<void>(first / 0.0);
    } catch (const std::domain_error&) {
        division_threw = true;
    }
    check(division_threw, "Vec2 division by zero throws");
}

void test_vec3() {
    using geometry::Vec3;

    const Vec3 zero;
    const Vec3 first{1.0, 2.0, 3.0};
    const Vec3 second{4.0, 5.0, 6.0};

    check(zero == Vec3{0.0, 0.0, 0.0}, "Vec3 default construction");
    check(first.x == 1.0 && first.y == 2.0 && first.z == 3.0, "Vec3 value construction");
    check(first + second == Vec3{5.0, 7.0, 9.0}, "Vec3 addition");
    check(second - first == Vec3{3.0, 3.0, 3.0}, "Vec3 subtraction");
    check(-first == Vec3{-1.0, -2.0, -3.0}, "Vec3 unary negation");
    check(first * 2.0 == Vec3{2.0, 4.0, 6.0}, "Vec3 right scalar multiplication");
    check(2.0 * first == Vec3{2.0, 4.0, 6.0}, "Vec3 left scalar multiplication");
    check(second / 2.0 == Vec3{2.0, 2.5, 3.0}, "Vec3 division");
    check(geometry::dot(first, second) == 32.0, "Vec3 dot product");
    check(geometry::cross(first, second) == Vec3{-3.0, 6.0, -3.0}, "Vec3 cross product");
    check(first.length_squared() == 14.0, "Vec3 squared length");
    check(near(Vec3{2.0, 3.0, 6.0}.length(), 7.0), "Vec3 length");

    const Vec3 normalized = Vec3{2.0, 3.0, 6.0}.normalized();
    check(near(normalized.x, 2.0 / 7.0) && near(normalized.y, 3.0 / 7.0) &&
              near(normalized.z, 6.0 / 7.0),
          "Vec3 normalization");
    check(zero.normalized() == zero, "Vec3 zero normalization");
    check(first != second, "Vec3 inequality");

    Vec3 compound{1.0, 2.0, 3.0};
    compound += second;
    compound -= Vec3{1.0, 1.0, 1.0};
    compound *= 2.0;
    compound /= 2.0;
    check(compound == Vec3{4.0, 6.0, 8.0}, "Vec3 compound operators");

    bool division_threw = false;
    try {
        static_cast<void>(first / -0.0);
    } catch (const std::domain_error&) {
        division_threw = true;
    }
    check(division_threw, "Vec3 division by zero throws");
}

}  // namespace

int main() {
    test_vec2();
    test_vec3();
    return failures == 0 ? 0 : 1;
}