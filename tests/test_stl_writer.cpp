#include "geometry/io/stl_writer.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, std::string_view description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

std::vector<char> read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(stream)),
                              std::istreambuf_iterator<char>());
}

void test_empty_mesh_produces_valid_header() {
    using namespace geometry;
    const Mesh mesh{};
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "empty_mesh.stl";
    io::write_binary_stl(mesh, path);

    const std::vector<char> data = read_file(path);
    check(data.size() == 84, "empty mesh produces 84-byte file");

    std::uint32_t count = 0;
    std::memcpy(&count, data.data() + 80, sizeof(count));
    check(count == 0, "empty mesh triangle count is 0");

    std::filesystem::remove(path);
}

void test_single_triangle_roundtrip() {
    using namespace geometry;
    const Mesh mesh{
        {Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0}},
        {TriangleIndices{0, 1, 2}},
    };
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "single_tri.stl";
    io::write_binary_stl(mesh, path);

    const std::vector<char> data = read_file(path);
    check(data.size() == 84 + 50, "single triangle produces 134-byte file");

    std::uint32_t count = 0;
    std::memcpy(&count, data.data() + 80, sizeof(count));
    check(count == 1, "single triangle count is 1");

    const Triangle triangle = mesh.triangle(0);
    const Vec3& normal = triangle.normal();

    float values[12] = {};
    std::memcpy(values, data.data() + 84, sizeof(values));

    check(values[0] == static_cast<float>(normal.x), "normal.x matches");
    check(values[1] == static_cast<float>(normal.y), "normal.y matches");
    check(values[2] == static_cast<float>(normal.z), "normal.z matches");

    check(values[3] == static_cast<float>(triangle.a().x), "vertex1.x matches");
    check(values[4] == static_cast<float>(triangle.a().y), "vertex1.y matches");
    check(values[5] == static_cast<float>(triangle.a().z), "vertex1.z matches");

    check(values[6] == static_cast<float>(triangle.b().x), "vertex2.x matches");
    check(values[7] == static_cast<float>(triangle.b().y), "vertex2.y matches");
    check(values[8] == static_cast<float>(triangle.b().z), "vertex2.z matches");

    check(values[9] == static_cast<float>(triangle.c().x), "vertex3.x matches");
    check(values[10] == static_cast<float>(triangle.c().y), "vertex3.y matches");
    check(values[11] == static_cast<float>(triangle.c().z), "vertex3.z matches");

    std::uint16_t attribute_byte_count = 0xFFFF;
    std::memcpy(&attribute_byte_count, data.data() + 84 + 48, sizeof(attribute_byte_count));
    check(attribute_byte_count == 0, "attribute byte count is 0");

    std::filesystem::remove(path);
}

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

void test_cube_mesh_produces_correct_size_and_count() {
    using namespace geometry;
    const Mesh cube = make_unit_cube();
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "cube_mesh.stl";
    io::write_binary_stl(cube, path);

    const std::vector<char> data = read_file(path);
    const std::size_t expected_size = 84 + 50 * cube.triangle_count();
    check(data.size() == expected_size, "cube mesh produces 84 + 50*12 byte file");

    std::uint32_t count = 0;
    std::memcpy(&count, data.data() + 80, sizeof(count));
    check(count == 12, "cube mesh triangle count is 12");

    std::filesystem::remove(path);
}

void test_binary_size_formula_holds_for_multiple_triangle_counts() {
    using namespace geometry;

    // Two triangles sharing an edge.
    const Mesh two_triangles{
        {Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{1.0, 1.0, 0.0},
         Point3{0.0, 1.0, 0.0}},
        {TriangleIndices{0, 1, 2}, TriangleIndices{0, 2, 3}},
    };
    const Mesh cube = make_unit_cube();

    for (const Mesh* mesh : {&two_triangles, &cube}) {
        const std::filesystem::path path =
            std::filesystem::temp_directory_path() / "size_formula.stl";
        io::write_binary_stl(*mesh, path);
        const std::vector<char> data = read_file(path);
        const std::size_t expected_size = 84 + 50 * mesh->triangle_count();
        check(data.size() == expected_size,
              "binary STL file size equals 84 + 50 * triangle_count");
        std::filesystem::remove(path);
    }
}

void test_winding_order_preserved() {
    using namespace geometry;
    // Reversed winding relative to the single-triangle test should produce an
    // opposite-facing normal, proving the writer does not reorder vertices.
    const Mesh mesh{
        {Point3{0.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0}, Point3{1.0, 0.0, 0.0}},
        {TriangleIndices{0, 1, 2}},
    };
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "winding.stl";
    io::write_binary_stl(mesh, path);

    const std::vector<char> data = read_file(path);
    const Triangle triangle = mesh.triangle(0);
    const Vec3& normal = triangle.normal();

    float values[3] = {};
    std::memcpy(values, data.data() + 84, sizeof(values));
    check(values[0] == static_cast<float>(normal.x), "reversed winding normal.x matches");
    check(values[1] == static_cast<float>(normal.y), "reversed winding normal.y matches");
    check(values[2] == static_cast<float>(normal.z), "reversed winding normal.z matches");
    check(normal.z < 0.0, "reversed winding faces opposite direction from CCW original");

    float vertex1[3] = {};
    std::memcpy(vertex1, data.data() + 84 + 12, sizeof(vertex1));
    check(vertex1[0] == static_cast<float>(triangle.a().x) &&
              vertex1[1] == static_cast<float>(triangle.a().y) &&
              vertex1[2] == static_cast<float>(triangle.a().z),
          "first written vertex matches first mesh vertex (winding order preserved)");

    std::filesystem::remove(path);
}

void test_invalid_path_throws() {
    using namespace geometry;
    const Mesh mesh{};
    bool threw = false;
    try {
        io::write_binary_stl(mesh, std::filesystem::path("/nonexistent_dir_xyz/out.stl"));
    } catch (const std::runtime_error&) {
        threw = true;
    }
    check(threw, "invalid path throws std::runtime_error");
}

}  // namespace

int main() {
    test_empty_mesh_produces_valid_header();
    test_single_triangle_roundtrip();
    test_cube_mesh_produces_correct_size_and_count();
    test_binary_size_formula_holds_for_multiple_triangle_counts();
    test_winding_order_preserved();
    test_invalid_path_throws();

    if (failures == 0) {
        std::cout << "All STL writer tests passed.\n";
        return 0;
    }
    std::cerr << failures << " STL writer test(s) failed.\n";
    return 1;
}
