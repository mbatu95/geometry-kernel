#pragma once

#include "geometry/vec3.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>

namespace geometry {

class Mat3 {
public:
    constexpr Mat3() noexcept = default;

    constexpr Mat3(double m00, double m01, double m02,
                   double m10, double m11, double m12,
                   double m20, double m21, double m22) noexcept
        : values_{m00, m01, m02, m10, m11, m12, m20, m21, m22} {}

    [[nodiscard]] static constexpr Mat3 identity() noexcept {
        return {1.0, 0.0, 0.0,
                0.0, 1.0, 0.0,
                0.0, 0.0, 1.0};
    }

    // Elements are stored row-major; invalid row or column indices throw.
    [[nodiscard]] constexpr double& operator()(std::size_t row, std::size_t column) {
        if (row >= 3 || column >= 3) {
            throw std::out_of_range("Mat3 index out of range");
        }
        return values_[row * 3 + column];
    }

    [[nodiscard]] constexpr const double& operator()(std::size_t row,
                                                      std::size_t column) const {
        if (row >= 3 || column >= 3) {
            throw std::out_of_range("Mat3 index out of range");
        }
        return values_[row * 3 + column];
    }

    [[nodiscard]] constexpr Mat3 operator+(const Mat3& other) const noexcept {
        Mat3 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] + other.values_[index];
        }
        return result;
    }

    [[nodiscard]] constexpr Mat3 operator-(const Mat3& other) const noexcept {
        Mat3 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] - other.values_[index];
        }
        return result;
    }

    [[nodiscard]] constexpr Mat3 operator*(double scalar) const noexcept {
        Mat3 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] * scalar;
        }
        return result;
    }

    [[nodiscard]] constexpr Mat3 operator/(double scalar) const {
        if (scalar == 0.0) {
            throw std::domain_error("Mat3 division by zero");
        }
        Mat3 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] / scalar;
        }
        return result;
    }

    [[nodiscard]] constexpr Mat3 operator*(const Mat3& other) const noexcept {
        Mat3 result;
        for (std::size_t row = 0; row < 3; ++row) {
            for (std::size_t column = 0; column < 3; ++column) {
                for (std::size_t inner = 0; inner < 3; ++inner) {
                    result.values_[row * 3 + column] +=
                        values_[row * 3 + inner] * other.values_[inner * 3 + column];
                }
            }
        }
        return result;
    }

    [[nodiscard]] constexpr Vec3 operator*(const Vec3& vector) const noexcept {
        return {
            values_[0] * vector.x + values_[1] * vector.y + values_[2] * vector.z,
            values_[3] * vector.x + values_[4] * vector.y + values_[5] * vector.z,
            values_[6] * vector.x + values_[7] * vector.y + values_[8] * vector.z,
        };
    }

    [[nodiscard]] constexpr Mat3 transposed() const noexcept {
        return {
            values_[0], values_[3], values_[6],
            values_[1], values_[4], values_[7],
            values_[2], values_[5], values_[8],
        };
    }

    constexpr Mat3& operator+=(const Mat3& other) noexcept {
        for (std::size_t index = 0; index < values_.size(); ++index) {
            values_[index] += other.values_[index];
        }
        return *this;
    }

    constexpr Mat3& operator-=(const Mat3& other) noexcept {
        for (std::size_t index = 0; index < values_.size(); ++index) {
            values_[index] -= other.values_[index];
        }
        return *this;
    }

    constexpr Mat3& operator*=(double scalar) noexcept {
        for (double& value : values_) {
            value *= scalar;
        }
        return *this;
    }

    constexpr Mat3& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("Mat3 division by zero");
        }
        for (double& value : values_) {
            value /= scalar;
        }
        return *this;
    }

    friend constexpr Mat3 operator*(double scalar, const Mat3& matrix) noexcept {
        return matrix * scalar;
    }

    friend constexpr bool operator==(const Mat3&, const Mat3&) noexcept = default;

    friend constexpr bool operator!=(const Mat3& lhs, const Mat3& rhs) noexcept {
        return !(lhs == rhs);
    }

private:
    std::array<double, 9> values_{};
};

}  // namespace geometry