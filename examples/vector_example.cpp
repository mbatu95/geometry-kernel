#include "geometry/point3.hpp"

#include <iostream>

int main() {
    const geometry::Point3 start{1.0, 2.0, 3.0};
    const geometry::Point3 end{4.0, 6.0, 3.0};
    const geometry::Vec3 displacement = end - start;
    const double length = geometry::distance(start, end);
    const geometry::Point3 moved = start + displacement;

    std::cout << "displacement: " << displacement.x << ", " << displacement.y << ", "
              << displacement.z << '\n';
    std::cout << "distance: " << length << '\n';
    std::cout << "moved point: " << moved.x << ", " << moved.y << ", " << moved.z << '\n';
}