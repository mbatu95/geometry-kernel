#include "geometry/intersection.hpp"

#include <iostream>

int main() {
    const geometry::Plane ground{
        geometry::Point3{0.0, 0.0, 0.0},
        geometry::Vec3{0.0, 0.0, 1.0},
    };
    const geometry::Ray ray{
        geometry::Point3{1.0, 2.0, 5.0},
        geometry::Vec3{0.0, 0.0, -1.0},
    };

    const auto hit = geometry::intersect(ray, ground);
    if (hit) {
        std::cout << "t: " << hit->t << '\n';
        std::cout << "intersection: " << hit->point.x << ", " << hit->point.y << ", "
                  << hit->point.z << '\n';
    } else {
        std::cout << "no intersection\n";
    }
}