#include "geometry/mat3.hpp"
#include "geometry/mat4.hpp"

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

bool near(const geometry::Mat4& actual, const geometry::Mat4& expected) {
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            if (!near(actual(row, column), expected(row, column))) {
                return false;
            }
        }
    }
    return true;
}

void test_mat3() {
    using geometry::Mat3;

    const Mat3 zero;
    const Mat3 matrix{1.0, 2.0, 3.0,
                      4.0, 5.0, 6.0,
                      7.0, 8.0, 9.0};
    const Mat3 other{9.0, 8.0, 7.0,
                     6.0, 5.0, 4.0,
                     3.0, 2.0, 1.0};
    const Mat3 identity = Mat3::identity();

    check(zero == Mat3{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
          "Mat3 zero construction");
    check(identity == Mat3{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0},
          "Mat3 identity");
    check(matrix(1, 2) == 6.0, "Mat3 element access");
    check(matrix + other == Mat3{10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0},
          "Mat3 addition");
    check(matrix - other == Mat3{-8.0, -6.0, -4.0, -2.0, 0.0, 2.0, 4.0, 6.0, 8.0},
          "Mat3 subtraction");
    check(matrix * 2.0 == Mat3{2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0},
          "Mat3 scalar multiplication");
    check(2.0 * matrix == matrix * 2.0, "Mat3 left scalar multiplication");
    check(matrix / 2.0 == Mat3{0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5},
          "Mat3 scalar division");
    check(matrix * other == Mat3{30.0, 24.0, 18.0, 84.0, 69.0, 54.0,
                                 138.0, 114.0, 90.0},
          "Mat3 matrix multiplication");
    check(identity * geometry::Vec3{1.0, 2.0, 3.0} == geometry::Vec3{1.0, 2.0, 3.0},
          "Mat3 times Vec3");
    check(matrix.transposed() == Mat3{1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0},
          "Mat3 transpose");
    check(matrix.transposed().transposed() == matrix, "Mat3 double transpose");
    check(identity * matrix == matrix && matrix * identity == matrix,
          "Mat3 identity multiplication");
    check(matrix == Mat3{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0},
          "Mat3 equality");
    check(matrix != other, "Mat3 inequality");
    check(near((matrix / 3.0)(2, 2), 3.0), "Mat3 approximate scalar result");

    Mat3 compound = matrix;
    compound += other;
    compound -= other;
    compound *= 2.0;
    compound /= 2.0;
    check(compound == matrix, "Mat3 compound operators");

    bool invalid_index_threw = false;
    try {
        static_cast<void>(matrix(3, 0));
    } catch (const std::out_of_range&) {
        invalid_index_threw = true;
    }
    check(invalid_index_threw, "Mat3 invalid index throws");

    bool division_threw = false;
    try {
        static_cast<void>(matrix / 0.0);
    } catch (const std::domain_error&) {
        division_threw = true;
    }
    check(division_threw, "Mat3 division by zero throws");
}

void test_mat4() {
    using geometry::Mat4;

    const Mat4 zero;
    const Mat4 matrix{1.0, 2.0, 3.0, 4.0,
                      5.0, 6.0, 7.0, 8.0,
                      9.0, 10.0, 11.0, 12.0,
                      13.0, 14.0, 15.0, 16.0};
    const Mat4 other{16.0, 15.0, 14.0, 13.0,
                     12.0, 11.0, 10.0, 9.0,
                     8.0, 7.0, 6.0, 5.0,
                     4.0, 3.0, 2.0, 1.0};
    const Mat4 identity = Mat4::identity();

    check(zero == Mat4{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
          "Mat4 zero construction");
    check(identity == Mat4{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0},
          "Mat4 identity");
    check(matrix(2, 3) == 12.0, "Mat4 element access");
    check(matrix + other == Mat4{17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0,
                                 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0},
          "Mat4 addition");
    check(matrix - other == Mat4{-15.0, -13.0, -11.0, -9.0, -7.0, -5.0, -3.0, -1.0,
                                 1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0},
          "Mat4 subtraction");
    check(matrix * 2.0 == Mat4{2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0,
                               18.0, 20.0, 22.0, 24.0, 26.0, 28.0, 30.0, 32.0},
          "Mat4 scalar multiplication");
    check(2.0 * matrix == matrix * 2.0, "Mat4 left scalar multiplication");
    check(matrix / 2.0 == Mat4{0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0,
                               4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0},
          "Mat4 scalar division");
    check(matrix * other == Mat4{80.0, 70.0, 60.0, 50.0, 240.0, 214.0, 188.0, 162.0,
                                 400.0, 358.0, 316.0, 274.0, 560.0, 502.0, 444.0, 386.0},
          "Mat4 matrix multiplication");
    check(matrix.transposed() == Mat4{1.0, 5.0, 9.0, 13.0, 2.0, 6.0, 10.0, 14.0,
                                      3.0, 7.0, 11.0, 15.0, 4.0, 8.0, 12.0, 16.0},
          "Mat4 transpose");
    check(matrix.transposed().transposed() == matrix, "Mat4 double transpose");
    check(identity * matrix == matrix && matrix * identity == matrix,
          "Mat4 identity multiplication");
    check(matrix == Mat4{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
                         9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0},
          "Mat4 equality");
    check(matrix != other, "Mat4 inequality");
    check(near((matrix / 3.0)(3, 3), 16.0 / 3.0), "Mat4 approximate scalar result");
    check(near(matrix * identity, matrix), "Mat4 approximate multiplication result");

    Mat4 compound = matrix;
    compound += other;
    compound -= other;
    compound *= 2.0;
    compound /= 2.0;
    check(compound == matrix, "Mat4 compound operators");

    bool invalid_index_threw = false;
    try {
        static_cast<void>(matrix(0, 4));
    } catch (const std::out_of_range&) {
        invalid_index_threw = true;
    }
    check(invalid_index_threw, "Mat4 invalid index throws");

    bool division_threw = false;
    try {
        static_cast<void>(matrix / -0.0);
    } catch (const std::domain_error&) {
        division_threw = true;
    }
    check(division_threw, "Mat4 division by zero throws");
}

}  // namespace

int main() {
    test_mat3();
    test_mat4();
    return failures == 0 ? 0 : 1;
}