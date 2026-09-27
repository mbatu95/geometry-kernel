#pragma once

#include "geometry/vec3.hpp"

#include <cmath>

namespace geometry {

struct Point3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    constexpr Point3() noexcept = default;
    constexpr Point3(double x_value, double y_value, double z_value) noexcept
        : x{x_value}, y{y_value}, z{z_value} {}

    [[nodiscard]] constexpr Vec3 operator-(const Point3& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Point3 operator+(const Vec3& displacement) const noexcept {
        return {x + displacement.x, y + displacement.y, z + displacement.z};
    }

    [[nodiscard]] constexpr Point3 operator-(const Vec3& displacement) const noexcept {
        return {x - displacement.x, y - displacement.y, z - displacement.z};
    }

    constexpr Point3& operator+=(const Vec3& displacement) noexcept {
        x += displacement.x;
        y += displacement.y;
        z += displacement.z;
        return *this;
    }

    constexpr Point3& operator-=(const Vec3& displacement) noexcept {
        x -= displacement.x;
        y -= displacement.y;
        z -= displacement.z;
        return *this;
    }

    friend constexpr bool operator==(const Point3&, const Point3&) noexcept = default;

    friend constexpr bool operator!=(const Point3& lhs, const Point3& rhs) noexcept {
        return !(lhs == rhs);
    }
};

[[nodiscard]] constexpr double distance_squared(const Point3& lhs, const Point3& rhs) noexcept {
    return (lhs - rhs).length_squared();
}

[[nodiscard]] inline double distance(const Point3& lhs, const Point3& rhs) noexcept {
    return std::sqrt(distance_squared(lhs, rhs));
}

}  // namespace geometry