#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

namespace geometry {

class Mat4 {
public:
    constexpr Mat4() noexcept = default;

    constexpr Mat4(double m00, double m01, double m02, double m03,
                   double m10, double m11, double m12, double m13,
                   double m20, double m21, double m22, double m23,
                   double m30, double m31, double m32, double m33) noexcept
        : values_{m00, m01, m02, m03,
                  m10, m11, m12, m13,
                  m20, m21, m22, m23,
                  m30, m31, m32, m33} {}

    [[nodiscard]] static constexpr Mat4 identity() noexcept {
        return {1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0};
    }

    // Elements are stored row-major; invalid row or column indices throw.
    [[nodiscard]] constexpr double& operator()(std::size_t row, std::size_t column) {
        if (row >= 4 || column >= 4) {
            throw std::out_of_range("Mat4 index out of range");
        }
        return values_[row * 4 + column];
    }

    [[nodiscard]] constexpr const double& operator()(std::size_t row,
                                                      std::size_t column) const {
        if (row >= 4 || column >= 4) {
            throw std::out_of_range("Mat4 index out of range");
        }
        return values_[row * 4 + column];
    }

    [[nodiscard]] constexpr Mat4 operator+(const Mat4& other) const noexcept {
        Mat4 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] + other.values_[index];
        }
        return result;
    }

    [[nodiscard]] constexpr Mat4 operator-(const Mat4& other) const noexcept {
        Mat4 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] - other.values_[index];
        }
        return result;
    }

    [[nodiscard]] constexpr Mat4 operator*(double scalar) const noexcept {
        Mat4 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] * scalar;
        }
        return result;
    }

    [[nodiscard]] constexpr Mat4 operator/(double scalar) const {
        if (scalar == 0.0) {
            throw std::domain_error("Mat4 division by zero");
        }
        Mat4 result;
        for (std::size_t index = 0; index < values_.size(); ++index) {
            result.values_[index] = values_[index] / scalar;
        }
        return result;
    }

    [[nodiscard]] constexpr Mat4 operator*(const Mat4& other) const noexcept {
        Mat4 result;
        for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t column = 0; column < 4; ++column) {
                for (std::size_t inner = 0; inner < 4; ++inner) {
                    result.values_[row * 4 + column] +=
                        values_[row * 4 + inner] * other.values_[inner * 4 + column];
                }
            }
        }
        return result;
    }

    [[nodiscard]] constexpr Mat4 transposed() const noexcept {
        return {
            values_[0], values_[4], values_[8], values_[12],
            values_[1], values_[5], values_[9], values_[13],
            values_[2], values_[6], values_[10], values_[14],
            values_[3], values_[7], values_[11], values_[15],
        };
    }

    constexpr Mat4& operator+=(const Mat4& other) noexcept {
        for (std::size_t index = 0; index < values_.size(); ++index) {
            values_[index] += other.values_[index];
        }
        return *this;
    }

    constexpr Mat4& operator-=(const Mat4& other) noexcept {
        for (std::size_t index = 0; index < values_.size(); ++index) {
            values_[index] -= other.values_[index];
        }
        return *this;
    }

    constexpr Mat4& operator*=(double scalar) noexcept {
        for (double& value : values_) {
            value *= scalar;
        }
        return *this;
    }

    constexpr Mat4& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("Mat4 division by zero");
        }
        for (double& value : values_) {
            value /= scalar;
        }
        return *this;
    }

    friend constexpr Mat4 operator*(double scalar, const Mat4& matrix) noexcept {
        return matrix * scalar;
    }

    friend constexpr bool operator==(const Mat4&, const Mat4&) noexcept = default;

    friend constexpr bool operator!=(const Mat4& lhs, const Mat4& rhs) noexcept {
        return !(lhs == rhs);
    }

private:
    std::array<double, 16> values_{};
};

}  // namespace geometry