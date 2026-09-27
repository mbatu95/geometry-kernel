#pragma once

#include "geometry/aabb.hpp"
#include "geometry/mesh.hpp"
#include "geometry/plane.hpp"
#include "geometry/ray.hpp"
#include "geometry/triangle.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
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

struct RayAABBIntersection {
    // Ray parameters for the first and last box contacts; t_enter is zero when
    // the ray origin is inside the box.
    double t_enter;
    double t_exit;
    Point3 enter_point;
    Point3 exit_point;
};

struct RayMeshIntersection {
    double t;
    Point3 point;
    std::size_t triangle_index;
    double u;
    double v;
};

[[nodiscard]] inline std::optional<RayAABBIntersection> intersect(
    const Ray& ray, const AABB& box) {
    const Point3& origin = ray.origin();
    const Vec3& direction = ray.direction();
    const Point3& minimum = box.min();
    const Point3& maximum = box.max();
    const double origins[] = {origin.x, origin.y, origin.z};
    const double directions[] = {direction.x, direction.y, direction.z};
    const double minima[] = {minimum.x, minimum.y, minimum.z};
    const double maxima[] = {maximum.x, maximum.y, maximum.z};

    double t_enter = -std::numeric_limits<double>::infinity();
    double t_exit = std::numeric_limits<double>::infinity();
    constexpr double interval_epsilon = 1e-12;

    for (int axis = 0; axis < 3; ++axis) {
        if (directions[axis] == 0.0) {
            if (origins[axis] < minima[axis] || origins[axis] > maxima[axis]) {
                return std::nullopt;
            }
            continue;
        }

        double axis_enter = (minima[axis] - origins[axis]) / directions[axis];
        double axis_exit = (maxima[axis] - origins[axis]) / directions[axis];
        if (axis_enter > axis_exit) {
            std::swap(axis_enter, axis_exit);
        }
        t_enter = std::max(t_enter, axis_enter);
        t_exit = std::min(t_exit, axis_exit);

        const double interval_scale = std::max({1.0, std::abs(t_enter), std::abs(t_exit)});
        if (t_enter > t_exit + interval_epsilon * interval_scale) {
            return std::nullopt;
        }
    }

    if (t_exit < 0.0) {
        return std::nullopt;
    }
    if (t_enter > t_exit) {
        const double contact = t_enter + (t_exit - t_enter) * 0.5;
        t_enter = contact;
        t_exit = contact;
    }

    t_enter = std::max(t_enter, 0.0);
    return RayAABBIntersection{
        t_enter, t_exit, ray.point_at(t_enter), ray.point_at(t_exit)};
}

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

[[nodiscard]] inline std::optional<RayMeshIntersection> intersect(
    const Ray& ray, const Mesh& mesh) {
    if (mesh.triangle_count() == 0) {
        return std::nullopt;
    }

    const std::optional<AABB> bounds = mesh.bounding_box();
    if (!bounds || !intersect(ray, *bounds)) {
        return std::nullopt;
    }

    std::optional<RayMeshIntersection> closest;
    for (std::size_t index = 0; index < mesh.triangle_count(); ++index) {
        const std::optional<RayTriangleIntersection> hit = intersect(ray, mesh.triangle(index));
        if (hit && (!closest || hit->t < closest->t)) {
            closest = RayMeshIntersection{hit->t, hit->point, index, hit->u, hit->v};
        }
    }
    return closest;
}

}  // namespace geometry