#include "geometry/intersection.hpp"

#include <iostream>
#include <string_view>

namespace {

void print_ray_result(std::string_view label, const geometry::Ray& ray,
                      const geometry::AABB& box) {
    const auto hit = geometry::intersect(ray, box);
    std::cout << label << ": ";
    if (!hit) {
        std::cout << "miss\n";
        return;
    }

    std::cout << "hit\n"
              << "  t_enter: " << hit->t_enter << "\n"
              << "  t_exit: " << hit->t_exit << "\n"
              << "  enter point: " << hit->enter_point.x << ", "
              << hit->enter_point.y << ", " << hit->enter_point.z << "\n"
              << "  exit point: " << hit->exit_point.x << ", "
              << hit->exit_point.y << ", " << hit->exit_point.z << '\n';
}

}  // namespace

int main() {
    using namespace geometry;

    const AABB box{Point3{0.0, 0.0, 0.0}, Point3{1.0, 1.0, 1.0}};
    print_ray_result("through", Ray{Point3{-1.0, 0.5, 0.5}, Vec3{1.0, 0.0, 0.0}}, box);
    print_ray_result("parallel miss", Ray{Point3{-1.0, 2.0, 0.5}, Vec3{1.0, 0.0, 0.0}}, box);
    print_ray_result("starting inside", Ray{Point3{0.5, 0.5, 0.5}, Vec3{0.0, 1.0, 0.0}}, box);
}