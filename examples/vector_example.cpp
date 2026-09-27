#include "geometry/vec3.hpp"

#include <iostream>

int main() {
    const geometry::Vec3 first{1.0, 2.0, 3.0};
    const geometry::Vec3 second{4.0, 5.0, 6.0};
    const geometry::Vec3 sum = first + second;
    const geometry::Vec3 product = geometry::cross(first, second);
    const geometry::Vec3 unit = first.normalized();

    std::cout << "sum: " << sum.x << ", " << sum.y << ", " << sum.z << '\n';
    std::cout << "dot: " << geometry::dot(first, second) << '\n';
    std::cout << "cross: " << product.x << ", " << product.y << ", " << product.z << '\n';
    std::cout << "normalized first: " << unit.x << ", " << unit.y << ", " << unit.z << '\n';
}