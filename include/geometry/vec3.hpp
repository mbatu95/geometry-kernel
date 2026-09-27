#pragma once

#include <cmath>
#include <stdexcept>

namespace geometry {

struct Vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    constexpr Vec3() noexcept = default;
    constexpr Vec3(double x_value, double y_value, double z_value) noexcept
        : x{x_value}, y{y_value}, z{z_value} {}

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-() const noexcept {
        return {-x, -y, -z};
    }

    [[nodiscard]] constexpr Vec3 operator*(double scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }

    [[nodiscard]] constexpr Vec3 operator/(double scalar) const {
        if (scalar == 0.0) {
            throw std::domain_error("Vec3 division by zero");
        }
        return {x / scalar, y / scalar, z / scalar};
    }

    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(double scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vec3& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("Vec3 division by zero");
        }
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr double length_squared() const noexcept {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] double length() const noexcept {
        return std::hypot(x, y, z);
    }

    // A zero-length vector normalizes to the zero vector.
    [[nodiscard]] Vec3 normalized() const {
        const double magnitude = length();
        return magnitude == 0.0 ? Vec3{} : *this / magnitude;
    }

    friend constexpr Vec3 operator*(double scalar, const Vec3& vector) noexcept {
        return vector * scalar;
    }

    friend constexpr bool operator==(const Vec3&, const Vec3&) noexcept = default;

    friend constexpr bool operator!=(const Vec3& lhs, const Vec3& rhs) noexcept {
        return !(lhs == rhs);
    }
};

[[nodiscard]] constexpr double dot(const Vec3& lhs, const Vec3& rhs) noexcept {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

[[nodiscard]] constexpr Vec3 cross(const Vec3& lhs, const Vec3& rhs) noexcept {
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

}  // namespace geometry