#include "geometry/vec2.hpp"
#include "geometry/vec3.hpp"
#include "geometry/point2.hpp"
#include "geometry/point3.hpp"

#include <cmath>
#include <concepts>
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

template <typename T>
concept AddableToSelf = requires(T lhs, T rhs) { lhs + rhs; };

template <typename T>
concept MultipliableByScalar = requires(T value) { value * 2.0; };

template <typename T>
concept DivisibleByScalar = requires(T value) { value / 2.0; };

static_assert(!AddableToSelf<geometry::Point2>);
static_assert(!AddableToSelf<geometry::Point3>);
static_assert(!MultipliableByScalar<geometry::Point2>);
static_assert(!MultipliableByScalar<geometry::Point3>);
static_assert(!DivisibleByScalar<geometry::Point2>);
static_assert(!DivisibleByScalar<geometry::Point3>);

void test_point2() {
    using geometry::Point2;
    using geometry::Vec2;

    const Point2 origin;
    const Point2 start{1.0, 2.0};
    const Point2 end{4.0, 6.0};
    const Vec2 displacement = end - start;

    check(origin == Point2{0.0, 0.0}, "Point2 default construction");
    check(start.x == 1.0 && start.y == 2.0, "Point2 value construction");
    check(displacement == Vec2{3.0, 4.0}, "Point2 subtraction produces Vec2");
    check(start + displacement == end, "Point2 plus Vec2");
    check(end - displacement == start, "Point2 minus Vec2");
    check(geometry::distance_squared(start, end) == 25.0, "Point2 squared distance");
    check(near(geometry::distance(start, end), 5.0), "Point2 distance");
    check(start == Point2{1.0, 2.0}, "Point2 equality");
    check(start != end, "Point2 inequality");

    Point2 translated = start;
    translated += displacement;
    translated -= Vec2{1.0, 2.0};
    check(translated == Point2{3.0, 4.0}, "Point2 compound translations");
}

void test_point3() {
    using geometry::Point3;
    using geometry::Vec3;

    const Point3 origin;
    const Point3 start{1.0, 2.0, 3.0};
    const Point3 end{4.0, 6.0, 3.0};
    const Vec3 displacement = end - start;

    check(origin == Point3{0.0, 0.0, 0.0}, "Point3 default construction");
    check(start.x == 1.0 && start.y == 2.0 && start.z == 3.0,
          "Point3 value construction");
    check(displacement == Vec3{3.0, 4.0, 0.0}, "Point3 subtraction produces Vec3");
    check(start + displacement == end, "Point3 plus Vec3");
    check(end - displacement == start, "Point3 minus Vec3");
    check(geometry::distance_squared(start, end) == 25.0, "Point3 squared distance");
    check(near(geometry::distance(start, end), 5.0), "Point3 distance");
    check(start == Point3{1.0, 2.0, 3.0}, "Point3 equality");
    check(start != end, "Point3 inequality");

    Point3 translated = start;
    translated += displacement;
    translated -= Vec3{1.0, 2.0, 3.0};
    check(translated == Point3{3.0, 4.0, 0.0}, "Point3 compound translations");
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
    test_point2();
    test_point3();
    return failures == 0 ? 0 : 1;
}