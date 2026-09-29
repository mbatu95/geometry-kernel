#pragma once

#include "geometry/mesh.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
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

inline void write_u16_le(std::ofstream& stream, std::uint16_t value,
                         const std::filesystem::path& path) {
    const std::array<std::uint8_t, 2> bytes{
        static_cast<std::uint8_t>(value & 0xFFu),
        static_cast<std::uint8_t>((value >> 8u) & 0xFFu),
    };
    write_bytes(stream, bytes.data(), bytes.size(), path);
}

inline void write_u32_le(std::ofstream& stream, std::uint32_t value,
                         const std::filesystem::path& path) {
    const std::array<std::uint8_t, 4> bytes{
        static_cast<std::uint8_t>(value & 0xFFu),
        static_cast<std::uint8_t>((value >> 8u) & 0xFFu),
        static_cast<std::uint8_t>((value >> 16u) & 0xFFu),
        static_cast<std::uint8_t>((value >> 24u) & 0xFFu),
    };
    write_bytes(stream, bytes.data(), bytes.size(), path);
}

inline void write_float32_le(std::ofstream& stream, double value,
                             const std::filesystem::path& path) {
    const float as_float = static_cast<float>(value);
    const std::uint32_t bits = std::bit_cast<std::uint32_t>(as_float);
    write_u32_le(stream, bits, path);
}

}  // namespace detail

// Writes the given mesh to a binary STL file at the given path.
//
// Binary STL stores all multi-byte numeric fields in little-endian order.
// Triangle winding order is preserved exactly as stored in the Mesh. Vertex and
// normal components are converted from double to float32 as required by the
// format. Throws std::overflow_error if the triangle count does not fit in a
// uint32_t, and std::runtime_error if the file cannot be opened or a write
// fails.
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

    const std::array<char, 80> header{};
    detail::write_bytes(stream, header.data(), header.size(), path);
    detail::write_u32_le(stream, static_cast<std::uint32_t>(triangle_count), path);

    for (std::size_t i = 0; i < triangle_count; ++i) {
        const Triangle triangle = mesh.triangle(i);
        const Vec3& normal = triangle.normal();

        detail::write_float32_le(stream, normal.x, path);
        detail::write_float32_le(stream, normal.y, path);
        detail::write_float32_le(stream, normal.z, path);

        for (const Point3* vertex : {&triangle.a(), &triangle.b(), &triangle.c()}) {
            detail::write_float32_le(stream, vertex->x, path);
            detail::write_float32_le(stream, vertex->y, path);
            detail::write_float32_le(stream, vertex->z, path);
        }

        detail::write_u16_le(stream, 0, path);
    }

    stream.flush();
    if (!stream) {
        throw std::runtime_error("Failed to flush STL data to file: " + path.string());
    }
}

}  // namespace geometry::io
