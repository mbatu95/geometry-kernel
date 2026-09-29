#pragma once

#include "geometry/aabb.hpp"
#include "geometry/transform.hpp"
#include "geometry/triangle.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace geometry {

struct TriangleIndices {
    std::size_t a;
    std::size_t b;
    std::size_t c;

    friend constexpr bool operator==(const TriangleIndices&, const TriangleIndices&) noexcept =
        default;
};

// Alias matching the design vocabulary; identical to TriangleIndices.
using TriangleIndex = TriangleIndices;

// Empty meshes are valid. Their bounding_box() is std::nullopt.
class Mesh {
public:
    Mesh(std::vector<Point3> vertices = {}, std::vector<TriangleIndices> triangles = {})
        : vertices_{std::move(vertices)}, triangles_{std::move(triangles)} {
        for (const TriangleIndices& indices : triangles_) {
            if (indices.a >= vertices_.size() || indices.b >= vertices_.size() ||
                indices.c >= vertices_.size()) {
                throw std::invalid_argument("Mesh triangle vertex index out of range");
            }
            static_cast<void>(triangle_from_indices(indices));
        }
    }

    [[nodiscard]] const std::vector<Point3>& vertices() const noexcept {
        return vertices_;
    }

    [[nodiscard]] const std::vector<TriangleIndices>& triangle_indices() const noexcept {
        return triangles_;
    }

    [[nodiscard]] std::size_t vertex_count() const noexcept {
        return vertices_.size();
    }

    [[nodiscard]] std::size_t triangle_count() const noexcept {
        return triangles_.size();
    }

    [[nodiscard]] Triangle triangle(std::size_t index) const {
        if (index >= triangles_.size()) {
            throw std::out_of_range("Mesh triangle index out of range");
        }
        return triangle_from_indices(triangles_[index]);
    }

    [[nodiscard]] std::optional<AABB> bounding_box() const {
        if (vertices_.empty()) {
            return std::nullopt;
        }

        Point3 minimum = vertices_.front();
        Point3 maximum = vertices_.front();
        for (const Point3& vertex : vertices_) {
            minimum.x = std::min(minimum.x, vertex.x);
            minimum.y = std::min(minimum.y, vertex.y);
            minimum.z = std::min(minimum.z, vertex.z);
            maximum.x = std::max(maximum.x, vertex.x);
            maximum.y = std::max(maximum.y, vertex.y);
            maximum.z = std::max(maximum.z, vertex.z);
        }
        return AABB{minimum, maximum};
    }

    [[nodiscard]] Mesh transformed(const Transform& transform) const {
        std::vector<Point3> transformed_vertices;
        transformed_vertices.reserve(vertices_.size());
        for (const Point3& vertex : vertices_) {
            transformed_vertices.push_back(transform.apply_point(vertex));
        }
        return Mesh{std::move(transformed_vertices), triangles_};
    }

private:
    [[nodiscard]] Triangle triangle_from_indices(const TriangleIndices& indices) const {
        return Triangle{vertices_[indices.a], vertices_[indices.b], vertices_[indices.c]};
    }

    std::vector<Point3> vertices_;
    std::vector<TriangleIndices> triangles_;
};

// Computes the axis-aligned bounding box over raw vertex positions.
// Returns std::nullopt when vertices is empty, matching Mesh::bounding_box().
[[nodiscard]] inline std::optional<AABB> compute_aabb(const std::vector<Point3>& vertices) {
    if (vertices.empty()) {
        return std::nullopt;
    }
    Point3 minimum = vertices.front();
    Point3 maximum = vertices.front();
    for (const Point3& vertex : vertices) {
        minimum.x = std::min(minimum.x, vertex.x);
        minimum.y = std::min(minimum.y, vertex.y);
        minimum.z = std::min(minimum.z, vertex.z);
        maximum.x = std::max(maximum.x, vertex.x);
        maximum.y = std::max(maximum.y, vertex.y);
        maximum.z = std::max(maximum.z, vertex.z);
    }
    return AABB{minimum, maximum};
}

[[nodiscard]] inline std::optional<AABB> compute_aabb(const Mesh& mesh) {
    return mesh.bounding_box();
}

// Computes a per-triangle normal from vertex positions and indices.
// Degenerate or out-of-range triangles produce a zero normal rather than throwing,
// so callers can combine this with has_degenerate_triangles() to validate raw data.
[[nodiscard]] inline std::vector<Vec3> compute_face_normals(
    const std::vector<Point3>& vertices, const std::vector<TriangleIndices>& triangles) {
    std::vector<Vec3> normals;
    normals.reserve(triangles.size());
    for (const TriangleIndices& indices : triangles) {
        if (indices.a >= vertices.size() || indices.b >= vertices.size() ||
            indices.c >= vertices.size()) {
            normals.push_back(Vec3{0.0, 0.0, 0.0});
            continue;
        }
        const Vec3 edge_ab = vertices[indices.b] - vertices[indices.a];
        const Vec3 edge_ac = vertices[indices.c] - vertices[indices.a];
        const Vec3 area_vector = cross(edge_ab, edge_ac);
        const double twice_area = area_vector.length();
        normals.push_back(twice_area == 0.0 ? Vec3{0.0, 0.0, 0.0} : area_vector / twice_area);
    }
    return normals;
}

[[nodiscard]] inline std::vector<Vec3> compute_face_normals(const Mesh& mesh) {
    std::vector<Vec3> normals;
    normals.reserve(mesh.triangle_count());
    for (std::size_t i = 0; i < mesh.triangle_count(); ++i) {
        normals.push_back(mesh.triangle(i).normal());
    }
    return normals;
}

// Detects triangles referencing out-of-range vertices or having zero area.
[[nodiscard]] inline bool has_degenerate_triangles(
    const std::vector<Point3>& vertices, const std::vector<TriangleIndices>& triangles) {
    for (const TriangleIndices& indices : triangles) {
        if (indices.a >= vertices.size() || indices.b >= vertices.size() ||
            indices.c >= vertices.size()) {
            return true;
        }
        const Vec3 edge_ab = vertices[indices.b] - vertices[indices.a];
        const Vec3 edge_ac = vertices[indices.c] - vertices[indices.a];
        if (cross(edge_ab, edge_ac).length() == 0.0) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool has_degenerate_triangles(const Mesh& mesh) {
    return has_degenerate_triangles(mesh.vertices(), mesh.triangle_indices());
}

// Checks that every undirected edge in the mesh is shared by exactly two triangles.
// An edge shared by more than two triangles (non-manifold geometry) is treated the
// same as an open boundary edge and makes the mesh non-watertight. A mesh with no
// triangles has no edges to violate this rule, so it is vacuously watertight.
[[nodiscard]] inline bool is_watertight(const std::vector<TriangleIndices>& triangles) {
    if (triangles.empty()) {
        return true;
    }
    std::map<std::pair<std::size_t, std::size_t>, int> edge_counts;
    auto add_edge = [&edge_counts](std::size_t first, std::size_t second) {
        const std::pair<std::size_t, std::size_t> key =
            first < second ? std::make_pair(first, second) : std::make_pair(second, first);
        ++edge_counts[key];
    };
    for (const TriangleIndices& indices : triangles) {
        add_edge(indices.a, indices.b);
        add_edge(indices.b, indices.c);
        add_edge(indices.c, indices.a);
    }
    for (const auto& [edge, count] : edge_counts) {
        static_cast<void>(edge);
        if (count != 2) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool is_watertight(const Mesh& mesh) {
    return is_watertight(mesh.triangle_indices());
}

}  // namespace geometry
