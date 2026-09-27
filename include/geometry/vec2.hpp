#pragma once

#include <cmath>
#include <stdexcept>

namespace geometry {

struct Vec2 {
    double x{0.0};
    double y{0.0};

    constexpr Vec2() noexcept = default;
    constexpr Vec2(double x_value, double y_value) noexcept : x{x_value}, y{y_value} {}

    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const noexcept {
        return {x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const noexcept {
        return {x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vec2 operator-() const noexcept {
        return {-x, -y};
    }

    [[nodiscard]] constexpr Vec2 operator*(double scalar) const noexcept {
        return {x * scalar, y * scalar};
    }

    [[nodiscard]] constexpr Vec2 operator/(double scalar) const {
        if (scalar == 0.0) {
            throw std::domain_error("Vec2 division by zero");
        }
        return {x / scalar, y / scalar};
    }

    constexpr Vec2& operator+=(const Vec2& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2& operator-=(const Vec2& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2& operator*=(double scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vec2& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("Vec2 division by zero");
        }
        x /= scalar;
        y /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr double length_squared() const noexcept {
        return x * x + y * y;
    }

    [[nodiscard]] double length() const noexcept {
        return std::hypot(x, y);
    }

    // A zero-length vector normalizes to the zero vector.
    [[nodiscard]] Vec2 normalized() const {
        const double magnitude = length();
        return magnitude == 0.0 ? Vec2{} : *this / magnitude;
    }

    friend constexpr Vec2 operator*(double scalar, const Vec2& vector) noexcept {
        return vector * scalar;
    }

    friend constexpr bool operator==(const Vec2&, const Vec2&) noexcept = default;

    friend constexpr bool operator!=(const Vec2& lhs, const Vec2& rhs) noexcept {
        return !(lhs == rhs);
    }
};

[[nodiscard]] constexpr double dot(const Vec2& lhs, const Vec2& rhs) noexcept {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

}  // namespace geometry