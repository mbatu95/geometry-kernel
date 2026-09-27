#include "geometry/transform.hpp"

#include <iostream>

int main() {
    const geometry::Point3 piece_position{0.0, 0.0, 0.0};
    const geometry::Transform move = geometry::Transform::translation(0.4, 0.2, 0.0);

    const geometry::Point3 world_position = move.apply_point(piece_position);
    const geometry::Vec3 direction{1.0, 0.0, 0.0};
    const geometry::Vec3 world_direction = move.apply_vector(direction);

    std::cout << "world position: " << world_position.x << ", " << world_position.y << ", "
              << world_position.z << '\n';
    std::cout << "world direction: " << world_direction.x << ", " << world_direction.y << ", "
              << world_direction.z << '\n';
}