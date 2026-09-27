#include "geometry/intersection.hpp"
#include "geometry/primitives.hpp"

#include <iostream>
#include <string_view>

namespace {

void print_mesh(std::string_view name, const geometry::Mesh& mesh) {
    std::cout << name << ": " << mesh.vertex_count() << " vertices, "
              << mesh.triangle_count() << " triangles";
    const auto bounds = mesh.bounding_box();
    if (bounds) {
        std::cout << ", bounds min (" << bounds->min().x << ", " << bounds->min().y
                  << ", " << bounds->min().z << "), max (" << bounds->max().x << ", "
                  << bounds->max().y << ", " << bounds->max().z << ")";
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    using namespace geometry;

    const Mesh box = make_box(2.0, 3.0, 1.0);
    const Mesh plane = make_xy_plane(10.0, 10.0);
    const Mesh grid = make_xy_grid(8.0, 8.0, 8, 8);
    print_mesh("box", box);
    print_mesh("plane", plane);
    print_mesh("grid", grid);

    const Mesh raised_box = box.transformed(Transform::translation(0.0, 0.0, 1.0));
    print_mesh("raised box", raised_box);

    const auto hit = intersect(
        Ray{Point3{0.0, 0.0, 3.0}, Vec3{0.0, 0.0, -1.0}}, box);
    if (hit) {
        std::cout << "ray hit box triangle " << hit->triangle_index << " at ("
                  << hit->point.x << ", " << hit->point.y << ", " << hit->point.z
                  << ")\n";
    } else {
        std::cout << "ray missed box\n";
    }
}