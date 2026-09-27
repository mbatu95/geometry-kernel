#pragma once

#include "geometry/plane.hpp"
#include "geometry/ray.hpp"
#include "geometry/triangle.hpp"

#include <cmath>
#include <optional>

namespace geometry {

struct RayPlaneIntersection {
    double t;
    Point3 point;
};

struct RayTriangleIntersection {
    double t;
    Point3 point;
    // Barycentric weights: A = 1 - u - v, B = u, C = v.
    double u;
    double v;
};

[[nodiscard]] inline std::optional<RayPlaneIntersection> intersect(
    const Ray& ray, const Plane& plane) {
    const Vec3& direction = ray.direction();
    const Vec3& normal = plane.normal();
    const double denominator = dot(normal, direction);

    // Dimensionless angular tolerance, scaled by direction length.
    constexpr double parallel_epsilon = 1e-12;
    if (std::abs(denominator) <= parallel_epsilon * direction.length()) {
        return std::nullopt;
    }

    const double t = dot(normal, plane.point() - ray.origin()) / denominator;
    if (t < 0.0) {
        return std::nullopt;
    }

    return RayPlaneIntersection{t, ray.point_at(t)};
}

[[nodiscard]] inline std::optional<RayTriangleIntersection> intersect(
    const Ray& ray, const Triangle& triangle) {
    const Vec3& direction = ray.direction();
    const Vec3 edge_ab = triangle.edge_ab();
    const Vec3 edge_ac = triangle.edge_ac();
    const Vec3 perpendicular = cross(direction, edge_ac);
    const double determinant = dot(edge_ab, perpendicular);

    // Scale the determinant threshold so ray-direction magnitude does not affect parallelism.
    constexpr double parallel_epsilon = 1e-12;
    const double determinant_scale = edge_ab.length() * direction.length() * edge_ac.length();
    if (std::abs(determinant) <= parallel_epsilon * determinant_scale) {
        return std::nullopt;
    }

    const double inverse_determinant = 1.0 / determinant;
    const Vec3 origin_offset = ray.origin() - triangle.a();
    const double u = dot(origin_offset, perpendicular) * inverse_determinant;

    constexpr double barycentric_epsilon = 1e-12;
    if (u < -barycentric_epsilon || u > 1.0 + barycentric_epsilon) {
        return std::nullopt;
    }

    const Vec3 cross_offset = cross(origin_offset, edge_ab);
    const double v = dot(direction, cross_offset) * inverse_determinant;
    if (v < -barycentric_epsilon || u + v > 1.0 + barycentric_epsilon) {
        return std::nullopt;
    }

    const double t = dot(edge_ac, cross_offset) * inverse_determinant;
    if (t < 0.0) {
        return std::nullopt;
    }

    return RayTriangleIntersection{t, ray.point_at(t), u, v};
}

}  // namespace geometry