#pragma once

#include "geometry/vec2.hpp"

#include <cmath>

namespace geometry {

struct Point2 {
    double x{0.0};
    double y{0.0};

    constexpr Point2() noexcept = default;
    constexpr Point2(double x_value, double y_value) noexcept : x{x_value}, y{y_value} {}

    [[nodiscard]] constexpr Vec2 operator-(const Point2& other) const noexcept {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Point2 operator+(const Vec2& displacement) const noexcept {
        return {x + displacement.x, y + displacement.y};
    }

    [[nodiscard]] constexpr Point2 operator-(const Vec2& displacement) const noexcept {
        return {x - displacement.x, y - displacement.y};
    }

    constexpr Point2& operator+=(const Vec2& displacement) noexcept {
        x += displacement.x;
        y += displacement.y;
        return *this;
    }

    constexpr Point2& operator-=(const Vec2& displacement) noexcept {
        x -= displacement.x;
        y -= displacement.y;
        return *this;
    }

    friend constexpr bool operator==(const Point2&, const Point2&) noexcept = default;

    friend constexpr bool operator!=(const Point2& lhs, const Point2& rhs) noexcept {
        return !(lhs == rhs);
    }
};

[[nodiscard]] constexpr double distance_squared(const Point2& lhs, const Point2& rhs) noexcept {
    return (lhs - rhs).length_squared();
}

[[nodiscard]] inline double distance(const Point2& lhs, const Point2& rhs) noexcept {
    return std::sqrt(distance_squared(lhs, rhs));
}

}  // namespace geometry