#pragma once

#include "geometry/mesh.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace geometry {
namespace detail {

inline void validate_primitive_dimension(double dimension, const char* name) {
    if (!std::isfinite(dimension) || dimension <= 0.0) {
        throw std::invalid_argument(std::string{name} + " must be finite and greater than zero");
    }
}

}  // namespace detail

[[nodiscard]] inline Mesh make_box(double width, double depth, double height) {
    detail::validate_primitive_dimension(width, "Box width");
    detail::validate_primitive_dimension(depth, "Box depth");
    detail::validate_primitive_dimension(height, "Box height");

    const double half_width = width * 0.5;
    const double half_depth = depth * 0.5;
    const double half_height = height * 0.5;

    return Mesh{
        {
            Point3{-half_width, -half_depth, -half_height},
            Point3{half_width, -half_depth, -half_height},
            Point3{half_width, half_depth, -half_height},
            Point3{-half_width, half_depth, -half_height},
            Point3{-half_width, -half_depth, half_height},
            Point3{half_width, -half_depth, half_height},
            Point3{half_width, half_depth, half_height},
            Point3{-half_width, half_depth, half_height},
        },
        {
            TriangleIndices{0, 2, 1}, TriangleIndices{0, 3, 2},
            TriangleIndices{4, 5, 6}, TriangleIndices{4, 6, 7},
            TriangleIndices{0, 1, 5}, TriangleIndices{0, 5, 4},
            TriangleIndices{1, 2, 6}, TriangleIndices{1, 6, 5},
            TriangleIndices{3, 7, 6}, TriangleIndices{3, 6, 2},
            TriangleIndices{0, 4, 7}, TriangleIndices{0, 7, 3},
        },
    };
}

[[nodiscard]] inline Mesh make_xy_plane(double width, double depth) {
    detail::validate_primitive_dimension(width, "Plane width");
    detail::validate_primitive_dimension(depth, "Plane depth");

    const double half_width = width * 0.5;
    const double half_depth = depth * 0.5;
    return Mesh{
        {
            Point3{-half_width, -half_depth, 0.0},
            Point3{half_width, -half_depth, 0.0},
            Point3{half_width, half_depth, 0.0},
            Point3{-half_width, half_depth, 0.0},
        },
        {TriangleIndices{0, 1, 2}, TriangleIndices{0, 2, 3}},
    };
}

[[nodiscard]] inline Mesh make_xy_grid(
    double width, double depth, std::size_t x_cells, std::size_t y_cells) {
    detail::validate_primitive_dimension(width, "Grid width");
    detail::validate_primitive_dimension(depth, "Grid depth");
    if (x_cells == 0 || y_cells == 0) {
        throw std::invalid_argument("Grid cell counts must be greater than zero");
    }

    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    if (x_cells == maximum || y_cells == maximum) {
        throw std::length_error("Grid dimensions exceed maximum mesh size");
    }
    const std::size_t x_vertices = x_cells + 1;
    const std::size_t y_vertices = y_cells + 1;
    if (x_vertices > maximum / y_vertices || x_cells > maximum / y_cells) {
        throw std::length_error("Grid dimensions exceed maximum mesh size");
    }
    const std::size_t vertex_count = x_vertices * y_vertices;
    const std::size_t cell_count = x_cells * y_cells;
    if (cell_count > maximum / 2) {
        throw std::length_error("Grid dimensions exceed maximum mesh size");
    }

    std::vector<Point3> vertices;
    vertices.reserve(vertex_count);
    const double half_width = width * 0.5;
    const double half_depth = depth * 0.5;
    const double x_step = width / static_cast<double>(x_cells);
    const double y_step = depth / static_cast<double>(y_cells);
    for (std::size_t y = 0; y <= y_cells; ++y) {
        const double y_position = y == y_cells
            ? half_depth
            : -half_depth + static_cast<double>(y) * y_step;
        for (std::size_t x = 0; x <= x_cells; ++x) {
            const double x_position = x == x_cells
                ? half_width
                : -half_width + static_cast<double>(x) * x_step;
            vertices.emplace_back(x_position, y_position, 0.0);
        }
    }

    std::vector<TriangleIndices> triangles;
    triangles.reserve(cell_count * 2);
    for (std::size_t y = 0; y < y_cells; ++y) {
        for (std::size_t x = 0; x < x_cells; ++x) {
            const std::size_t lower_left = y * x_vertices + x;
            const std::size_t lower_right = lower_left + 1;
            const std::size_t upper_left = lower_left + x_vertices;
            const std::size_t upper_right = upper_left + 1;
            triangles.push_back({lower_left, lower_right, upper_right});
            triangles.push_back({lower_left, upper_right, upper_left});
        }
    }

    return Mesh{std::move(vertices), std::move(triangles)};
}

}  // namespace geometry