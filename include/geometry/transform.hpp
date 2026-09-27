#pragma once

#include "geometry/mat4.hpp"
#include "geometry/point3.hpp"
#include "geometry/vec3.hpp"

#include <cmath>
#include <stdexcept>

namespace geometry {

class Transform {
public:
    constexpr Transform() noexcept = default;

    // Only matrices with an affine bottom row [0, 0, 0, 1] are accepted.
    explicit Transform(const Mat4& matrix) : matrix_{matrix} {
        if (matrix_(3, 0) != 0.0 || matrix_(3, 1) != 0.0 ||
            matrix_(3, 2) != 0.0 || matrix_(3, 3) != 1.0) {
            throw std::invalid_argument("Transform matrix must be affine");
        }
    }

    [[nodiscard]] static constexpr Transform identity() noexcept {
        return {};
    }

    [[nodiscard]] static Transform translation(const Vec3& offset) {
        return Transform{Mat4{
            1.0, 0.0, 0.0, offset.x,
            0.0, 1.0, 0.0, offset.y,
            0.0, 0.0, 1.0, offset.z,
            0.0, 0.0, 0.0, 1.0,
        }};
    }

    [[nodiscard]] static Transform translation(double x, double y, double z) {
        return translation(Vec3{x, y, z});
    }

    [[nodiscard]] static Transform scaling(double uniform_scale) {
        return scaling(uniform_scale, uniform_scale, uniform_scale);
    }

    [[nodiscard]] static Transform scaling(double sx, double sy, double sz) {
        return Transform{Mat4{
            sx,  0.0, 0.0, 0.0,
            0.0, sy,  0.0, 0.0,
            0.0, 0.0, sz,  0.0,
            0.0, 0.0, 0.0, 1.0,
        }};
    }

    [[nodiscard]] static Transform rotation_x(double radians) {
        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);
        return Transform{Mat4{
            1.0, 0.0,    0.0,   0.0,
            0.0, cosine, -sine, 0.0,
            0.0, sine,   cosine, 0.0,
            0.0, 0.0,    0.0,   1.0,
        }};
    }

    [[nodiscard]] static Transform rotation_y(double radians) {
        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);
        return Transform{Mat4{
            cosine, 0.0, sine,   0.0,
            0.0,    1.0, 0.0,    0.0,
            -sine,  0.0, cosine, 0.0,
            0.0,    0.0, 0.0,    1.0,
        }};
    }

    [[nodiscard]] static Transform rotation_z(double radians) {
        const double cosine = std::cos(radians);
        const double sine = std::sin(radians);
        return Transform{Mat4{
            cosine, -sine, 0.0, 0.0,
            sine,   cosine, 0.0, 0.0,
            0.0,    0.0,    1.0, 0.0,
            0.0,    0.0,    0.0, 1.0,
        }};
    }

    [[nodiscard]] Point3 apply_point(const Point3& point) const {
        return {
            matrix_(0, 0) * point.x + matrix_(0, 1) * point.y +
                matrix_(0, 2) * point.z + matrix_(0, 3),
            matrix_(1, 0) * point.x + matrix_(1, 1) * point.y +
                matrix_(1, 2) * point.z + matrix_(1, 3),
            matrix_(2, 0) * point.x + matrix_(2, 1) * point.y +
                matrix_(2, 2) * point.z + matrix_(2, 3),
        };
    }

    [[nodiscard]] Vec3 apply_vector(const Vec3& vector) const {
        return {
            matrix_(0, 0) * vector.x + matrix_(0, 1) * vector.y + matrix_(0, 2) * vector.z,
            matrix_(1, 0) * vector.x + matrix_(1, 1) * vector.y + matrix_(1, 2) * vector.z,
            matrix_(2, 0) * vector.x + matrix_(2, 1) * vector.y + matrix_(2, 2) * vector.z,
        };
    }

    // A * B applies B first, then A, consistent with column-vector multiplication.
    [[nodiscard]] Transform operator*(const Transform& other) const {
        return Transform{matrix_ * other.matrix_};
    }

    [[nodiscard]] const Mat4& matrix() const noexcept {
        return matrix_;
    }

private:
    Mat4 matrix_{Mat4::identity()};
};

}  // namespace geometry