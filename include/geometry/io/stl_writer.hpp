#pragma once

#include "geometry/mesh.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

namespace geometry::io {

namespace detail {

inline void write_bytes(std::ofstream& stream, const void* data, std::size_t size,
                         const std::filesystem::path& path) {
    stream.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    if (!stream) {
        throw std::runtime_error("Failed to write STL data to file: " + path.string());
    }
}

inline void write_float(std::ofstream& stream, double value, const std::filesystem::path& path) {
    const float as_float = static_cast<float>(value);
    write_bytes(stream, &as_float, sizeof(as_float), path);
}

}  // namespace detail

// Writes the given mesh to a binary STL file at the given path.
//
// Triangle winding order is preserved exactly as stored in the Mesh. Vertex and
// normal components are converted from double to float32 as required by the
// binary STL format. Throws std::overflow_error if the triangle count does not
// fit in a uint32_t, and std::runtime_error if the file cannot be opened or a
// write fails.
inline void write_binary_stl(const Mesh& mesh, const std::filesystem::path& path) {
    const std::size_t triangle_count = mesh.triangle_count();
    if (triangle_count > std::numeric_limits<std::uint32_t>::max()) {
        throw std::overflow_error(
            "Mesh triangle count exceeds the maximum representable in a binary STL file (uint32_t)");
    }

    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        throw std::runtime_error("Failed to open file for writing STL: " + path.string());
    }

    // 80-byte header, zero-filled.
    std::array<char, 80> header{};
    detail::write_bytes(stream, header.data(), header.size(), path);

    const auto count32 = static_cast<std::uint32_t>(triangle_count);
    detail::write_bytes(stream, &count32, sizeof(count32), path);

    for (std::size_t i = 0; i < triangle_count; ++i) {
        const Triangle triangle = mesh.triangle(i);
        const Vec3& normal = triangle.normal();

        detail::write_float(stream, normal.x, path);
        detail::write_float(stream, normal.y, path);
        detail::write_float(stream, normal.z, path);

        for (const Point3* vertex : {&triangle.a(), &triangle.b(), &triangle.c()}) {
            detail::write_float(stream, vertex->x, path);
            detail::write_float(stream, vertex->y, path);
            detail::write_float(stream, vertex->z, path);
        }

        const std::uint16_t attribute_byte_count = 0;
        detail::write_bytes(stream, &attribute_byte_count, sizeof(attribute_byte_count), path);
    }

    stream.flush();
    if (!stream) {
        throw std::runtime_error("Failed to flush STL data to file: " + path.string());
    }
}

}  // namespace geometry::io
