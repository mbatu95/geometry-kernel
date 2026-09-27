#pragma once

#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

#include <stdexcept>

namespace geometry {

class Triangle {
public:
    Triangle(Point3 a, Point3 b, Point3 c) : a_{a}, b_{b}, c_{c} {
        const Vec3 area_vector = cross(edge_ab(), edge_ac());
        const double twice_area = area_vector.length();
        if (twice_area == 0.0) {
            throw std::invalid_argument("Triangle vertices must not be collinear");
        }
        area_ = 0.5 * twice_area;
        normal_ = area_vector / twice_area;
    }

    [[nodiscard]] const Point3& a() const noexcept {
        return a_;
    }

    [[nodiscard]] const Point3& b() const noexcept {
        return b_;
    }

    [[nodiscard]] const Point3& c() const noexcept {
        return c_;
    }

    [[nodiscard]] constexpr Vec3 edge_ab() const noexcept {
        return b_ - a_;
    }

    [[nodiscard]] constexpr Vec3 edge_ac() const noexcept {
        return c_ - a_;
    }

    [[nodiscard]] const Vec3& normal() const noexcept {
        return normal_;
    }

    [[nodiscard]] double area() const noexcept {
        return area_;
    }

private:
    Point3 a_;
    Point3 b_;
    Point3 c_;
    Vec3 normal_{};
    double area_{0.0};
};

}  // namespace geometry