#include "geometry/mat3.hpp"
#include "geometry/mat4.hpp"

#include <iostream>

int main() {
    const geometry::Mat3 identity = geometry::Mat3::identity();
    const geometry::Vec3 vector{1.0, 2.0, 3.0};
    const geometry::Vec3 result = identity * vector;

    const geometry::Mat4 matrix{1.0, 2.0, 3.0, 4.0,
                                5.0, 6.0, 7.0, 8.0,
                                9.0, 10.0, 11.0, 12.0,
                                13.0, 14.0, 15.0, 16.0};
    const geometry::Mat4 product = geometry::Mat4::identity() * matrix;

    std::cout << "Mat3 * Vec3: " << result.x << ", " << result.y << ", " << result.z << '\n';
    std::cout << "Mat4 product entries: " << product(0, 0) << ", " << product(3, 3) << '\n';
}