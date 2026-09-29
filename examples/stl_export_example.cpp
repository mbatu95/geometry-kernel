// Demonstrates constructing a simple cube Mesh and exporting it to a binary
// STL file on disk using geometry::io::write_binary_stl.
#include "geometry/io/stl_writer.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

namespace {

geometry::Mesh make_unit_cube() {
    using namespace geometry;
    // A closed, watertight unit cube spanning [0,1]^3 with outward-facing windings.
    const std::vector<Point3> vertices{
        Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{1.0, 1.0, 0.0},
        Point3{0.0, 1.0, 0.0}, Point3{0.0, 0.0, 1.0}, Point3{1.0, 0.0, 1.0},
        Point3{1.0, 1.0, 1.0}, Point3{0.0, 1.0, 1.0},
    };
    const std::vector<TriangleIndices> triangles{
        // -z face
        TriangleIndices{0, 2, 1}, TriangleIndices{0, 3, 2},
        // +z face
        TriangleIndices{4, 5, 6}, TriangleIndices{4, 6, 7},
        // -y face
        TriangleIndices{0, 1, 5}, TriangleIndices{0, 5, 4},
        // +y face
        TriangleIndices{3, 7, 6}, TriangleIndices{3, 6, 2},
        // -x face
        TriangleIndices{0, 4, 7}, TriangleIndices{0, 7, 3},
        // +x face
        TriangleIndices{1, 2, 6}, TriangleIndices{1, 6, 5},
    };
    return Mesh{vertices, triangles};
}

}  // namespace

int main() {
    using namespace geometry;

    const Mesh cube = make_unit_cube();

    const std::filesystem::path output_path = std::filesystem::current_path() / "box.stl";
    io::write_binary_stl(cube, output_path);

    std::cout << "Exported cube mesh with " << cube.vertex_count() << " vertices and "
              << cube.triangle_count() << " triangles to " << output_path.string() << '\n';
}
