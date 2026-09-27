#pragma once

#include "geometry/point3.hpp"
#include "geometry/triangle.hpp"
#include "geometry/vec3.hpp"

#include <algorithm>
#include <stdexcept>

namespace geometry {

class AABB {
public:
    AABB(Point3 minimum, Point3 maximum) : min_{minimum}, max_{maximum} {
        if (!(min_.x <= max_.x && min_.y <= max_.y && min_.z <= max_.z)) {
            throw std::invalid_argument("AABB minimum bounds must not exceed maximum bounds");
        }
    }

    [[nodiscard]] static AABB from_triangle(const Triangle& triangle) {
        const Point3& a = triangle.a();
        const Point3& b = triangle.b();
        const Point3& c = triangle.c();
        return {
            Point3{std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y}),
                   std::min({a.z, b.z, c.z})},
            Point3{std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y}),
                   std::max({a.z, b.z, c.z})},
        };
    }

    [[nodiscard]] const Point3& min() const noexcept {
        return min_;
    }

    [[nodiscard]] const Point3& max() const noexcept {
        return max_;
    }

    [[nodiscard]] Point3 center() const noexcept {
        return min_ + size() * 0.5;
    }

    [[nodiscard]] Vec3 size() const noexcept {
        return max_ - min_;
    }

    [[nodiscard]] bool contains(const Point3& point) const noexcept {
        return point.x >= min_.x && point.x <= max_.x && point.y >= min_.y &&
               point.y <= max_.y && point.z >= min_.z && point.z <= max_.z;
    }

    void expand(const Point3& point) noexcept {
        min_.x = std::min(min_.x, point.x);
        min_.y = std::min(min_.y, point.y);
        min_.z = std::min(min_.z, point.z);
        max_.x = std::max(max_.x, point.x);
        max_.y = std::max(max_.y, point.y);
        max_.z = std::max(max_.z, point.z);
    }

private:
    Point3 min_;
    Point3 max_;
};

}  // namespace geometry