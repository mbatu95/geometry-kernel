#include "geometry/io/stl_writer.hpp"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

std::vector<std::uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    const std::vector<char> raw((std::istreambuf_iterator<char>(stream)),
                                std::istreambuf_iterator<char>());
    return {raw.begin(), raw.end()};
}

std::uint16_t read_u16_le(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(data[offset]) |
        (static_cast<std::uint16_t>(data[offset + 1]) << 8u));
}

std::uint32_t read_u32_le(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return static_cast<std::uint32_t>(data[offset]) |
           (static_cast<std::uint32_t>(data[offset + 1]) << 8u) |
           (static_cast<std::uint32_t>(data[offset + 2]) << 16u) |
           (static_cast<std::uint32_t>(data[offset + 3]) << 24u);
}

float read_float32_le(const std::vector<std::uint8_t>& data, std::size_t offset) {
    return std::bit_cast<float>(read_u32_le(data, offset));
}

bool nearly_equal(float lhs, float rhs, float tolerance = 1.0e-6F) {
    return std::fabs(lhs - rhs) <= tolerance;
}

void test_empty_mesh_produces_valid_header() {
    using namespace geometry;
    const Mesh mesh{};
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "empty_mesh.stl";
    io::write_binary_stl(mesh, path);

    const auto data = read_file(path);
    check(data.size() == 84, "empty mesh produces 84-byte file");
    check(read_u32_le(data, 80) == 0, "empty mesh triangle count is 0");

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

    const auto data = read_file(path);
    check(data.size() == 134, "single triangle produces 134-byte file");
    check(read_u32_le(data, 80) == 1, "single triangle count is 1");

    const Triangle triangle = mesh.triangle(0);
    const Vec3& normal = triangle.normal();

    const float expected[12] = {
        static_cast<float>(normal.x), static_cast<float>(normal.y), static_cast<float>(normal.z),
        static_cast<float>(triangle.a().x), static_cast<float>(triangle.a().y),
        static_cast<float>(triangle.a().z), static_cast<float>(triangle.b().x),
        static_cast<float>(triangle.b().y), static_cast<float>(triangle.b().z),
        static_cast<float>(triangle.c().x), static_cast<float>(triangle.c().y),
        static_cast<float>(triangle.c().z),
    };

    for (std::size_t i = 0; i < 12; ++i) {
        check(nearly_equal(read_float32_le(data, 84 + i * 4), expected[i]),
              "single triangle float32 field matches expected little-endian value");
    }

    check(read_u16_le(data, 84 + 48) == 0, "attribute byte count is 0");

    // Exact bytes for 1.0f must be 00 00 80 3F in little-endian order.
    check(data[84 + 3 * 4] == 0x00 && data[84 + 3 * 4 + 1] == 0x00 &&
              data[84 + 3 * 4 + 2] == 0x00 && data[84 + 3 * 4 + 3] == 0x00,
          "vertex1.x 0.0f uses expected little-endian bytes");
    check(data[84 + 6 * 4] == 0x00 && data[84 + 6 * 4 + 1] == 0x00 &&
              data[84 + 6 * 4 + 2] == 0x80 && data[84 + 6 * 4 + 3] == 0x3F,
          "vertex2.x 1.0f uses expected little-endian bytes");

    std::filesystem::remove(path);
}

geometry::Mesh make_unit_cube() {
    using namespace geometry;
    const std::vector<Point3> vertices{
        Point3{0.0, 0.0, 0.0}, Point3{1.0, 0.0, 0.0}, Point3{1.0, 1.0, 0.0},
        Point3{0.0, 1.0, 0.0}, Point3{0.0, 0.0, 1.0}, Point3{1.0, 0.0, 1.0},
        Point3{1.0, 1.0, 1.0}, Point3{0.0, 1.0, 1.0},
    };
    const std::vector<TriangleIndices> triangles{
        TriangleIndices{0, 2, 1}, TriangleIndices{0, 3, 2},
        TriangleIndices{4, 5, 6}, TriangleIndices{4, 6, 7},
        TriangleIndices{0, 1, 5}, TriangleIndices{0, 5, 4},
        TriangleIndices{3, 7, 6}, TriangleIndices{3, 6, 2},
        TriangleIndices{0, 4, 7}, TriangleIndices{0, 7, 3},
        TriangleIndices{1, 2, 6}, TriangleIndices{1, 6, 5},
    };
    return Mesh{vertices, triangles};
}

void test_cube_mesh_produces_correct_size_and_count() {
    using namespace geometry;
    const Mesh cube = make_unit_cube();
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "cube_mesh.stl";
    io::write_binary_stl(cube, path);

    const auto data = read_file(path);
    check(data.size() == 84 + 50 * cube.triangle_count(),
          "cube mesh produces 84 + 50*12 byte file");
    check(read_u32_le(data, 80) == 12, "cube mesh triangle count is 12");

    std::filesystem::remove(path);
}

void test_binary_size_formula_holds_for_multiple_triangle_counts() {
    using namespace geometry;

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
        const auto data = read_file(path);
        check(data.size() == 84 + 50 * mesh->triangle_count(),
              "binary STL file size equals 84 + 50 * triangle_count");
        check(read_u32_le(data, 80) == mesh->triangle_count(),
              "triangle count decodes from explicit little-endian bytes");
        std::filesystem::remove(path);
    }
}

void test_winding_order_preserved() {
    using namespace geometry;
    const Mesh mesh{
        {Point3{0.0, 0.0, 0.0}, Point3{0.0, 1.0, 0.0}, Point3{1.0, 0.0, 0.0}},
        {TriangleIndices{0, 1, 2}},
    };
    const std::filesystem::path path = std::filesystem::temp_directory_path() / "winding.stl";
    io::write_binary_stl(mesh, path);

    const auto data = read_file(path);
    const Triangle triangle = mesh.triangle(0);
    const Vec3& normal = triangle.normal();

    check(nearly_equal(read_float32_le(data, 84), static_cast<float>(normal.x)),
          "reversed winding normal.x matches");
    check(nearly_equal(read_float32_le(data, 88), static_cast<float>(normal.y)),
          "reversed winding normal.y matches");
    check(nearly_equal(read_float32_le(data, 92), static_cast<float>(normal.z)),
          "reversed winding normal.z matches");
    check(normal.z < 0.0, "reversed winding faces opposite direction from CCW original");

    check(nearly_equal(read_float32_le(data, 96), static_cast<float>(triangle.a().x)) &&
              nearly_equal(read_float32_le(data, 100), static_cast<float>(triangle.a().y)) &&
              nearly_equal(read_float32_le(data, 104), static_cast<float>(triangle.a().z)),
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
