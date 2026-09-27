#include "geometry/intersection.hpp"

#include <iostream>

namespace {

void print_bounds(const geometry::Mesh& mesh) {
    const auto bounds = mesh.bounding_box();
    if (!bounds) {
        std::cout << "bounding box: empty\n";
        return;
    }
    std::cout << "bounding box: min (" << bounds->min().x << ", " << bounds->min().y
              << ", " << bounds->min().z << "), max (" << bounds->max().x << ", "
              << bounds->max().y << ", " << bounds->max().z << ")\n";
}

}  // namespace

int main() {
    using namespace geometry;

    const Mesh mesh{
        {
            Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0},
            Point3{1.0, 1.0, 0.0}, Point3{0.0, 1.0, 0.0},
        },
        {TriangleIndices{0, 1, 2}, TriangleIndices{0, 2, 3}},
    };

    std::cout << "vertices: " << mesh.vertex_count() << '\n'
              << "triangles: " << mesh.triangle_count() << '\n';
    print_bounds(mesh);

    const Ray ray{Point3{0.75, 0.25, 2.0}, Vec3{0.0, 0.0, -1.0}};
    const auto hit = intersect(ray, mesh);
    if (hit) {
        std::cout << "ray hit triangle " << hit->triangle_index << " at (" << hit->point.x
                  << ", " << hit->point.y << ", " << hit->point.z << ")\n";
    } else {
        std::cout << "ray missed mesh\n";
    }

    const Mesh translated = mesh.transformed(Transform::translation(2.0, 0.0, 1.0));
    std::cout << "original first vertex: (" << mesh.vertices()[0].x << ", "
              << mesh.vertices()[0].y << ", " << mesh.vertices()[0].z << ")\n"
              << "translated first vertex: (" << translated.vertices()[0].x << ", "
              << translated.vertices()[0].y << ", " << translated.vertices()[0].z << ")\n";
    print_bounds(translated);
}