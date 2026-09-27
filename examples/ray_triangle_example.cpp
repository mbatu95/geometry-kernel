#include "geometry/intersection.hpp"

#include <iostream>

int main() {
    const geometry::Triangle triangle{
        geometry::Point3{0.0, 0.0, 0.0},
        geometry::Point3{2.0, 0.0, 0.0},
        geometry::Point3{0.0, 2.0, 0.0},
    };
    const geometry::Ray ray{
        geometry::Point3{0.5, 0.5, 1.0},
        geometry::Vec3{0.0, 0.0, -1.0},
    };

    const auto hit = geometry::intersect(ray, triangle);
    if (hit) {
        std::cout << "intersection: " << hit->point.x << ", " << hit->point.y << ", "
                  << hit->point.z << '\n';
        std::cout << "t: " << hit->t << '\n';
        std::cout << "u: " << hit->u << '\n';
        std::cout << "v: " << hit->v << '\n';
    } else {
        std::cout << "no intersection\n";
    }
}