#pragma once

#include "geometry/aabb.hpp"
#include "geometry/transform.hpp"
#include "geometry/triangle.hpp"

#include <algorithm>
#include <cstddef>
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

}  // namespace geometry