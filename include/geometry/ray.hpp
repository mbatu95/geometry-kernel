#pragma once

#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

#include <stdexcept>

namespace geometry {

class Ray {
public:
    Ray(Point3 origin, Vec3 direction)
        : origin_{origin}, direction_{direction} {
        if (direction_.length() == 0.0) {
            throw std::invalid_argument("Ray direction must be non-zero");
        }
    }

    [[nodiscard]] const Point3& origin() const noexcept {
        return origin_;
    }

    [[nodiscard]] const Vec3& direction() const noexcept {
        return direction_;
    }

    // The direction is not normalized, so t scales inversely with its magnitude.
    [[nodiscard]] Point3 point_at(double t) const noexcept {
        return origin_ + direction_ * t;
    }

private:
    Point3 origin_;
    Vec3 direction_;
};

}  // namespace geometry