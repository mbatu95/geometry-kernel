#pragma once

#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

#include <stdexcept>

namespace geometry {

class Plane {
public:
    Plane(Point3 point, Vec3 normal) : point_{point}, normal_{normal.normalized()} {
        if (normal.length() == 0.0) {
            throw std::invalid_argument("Plane normal must be non-zero");
        }
    }

    [[nodiscard]] const Point3& point() const noexcept {
        return point_;
    }

    [[nodiscard]] const Vec3& normal() const noexcept {
        return normal_;
    }

    [[nodiscard]] double signed_distance(const Point3& query) const noexcept {
        return dot(normal_, query - point_);
    }

private:
    Point3 point_;
    Vec3 normal_;
};

}  // namespace geometry