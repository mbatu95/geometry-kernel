#pragma once

#include "geometry/plane.hpp"
#include "geometry/ray.hpp"

#include <cmath>
#include <optional>

namespace geometry {

struct RayPlaneIntersection {
    double t;
    Point3 point;
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

}  // namespace geometry