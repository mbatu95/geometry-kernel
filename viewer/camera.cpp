#include "viewer/camera.hpp"

#include "geometry/vec3.hpp"

#include <algorithm>
#include <cmath>

namespace viewer {
namespace {

constexpr double pi = 3.14159265358979323846;
constexpr geometry::Vec3 world_up{0.0, 0.0, 1.0};

geometry::Point3 camera_position(
    const geometry::Point3& target, double yaw, double pitch, double distance) {
    const double horizontal_distance = distance * std::cos(pitch);
    return target + geometry::Vec3{
        horizontal_distance * std::cos(yaw),
        horizontal_distance * std::sin(yaw),
        distance * std::sin(pitch),
    };
}

}  // namespace

geometry::Mat4 Camera::view_matrix() const {
    const geometry::Point3 position = camera_position(target_, yaw_, pitch_, distance_);
    const geometry::Vec3 forward = (target_ - position).normalized();
    const geometry::Vec3 right = geometry::cross(forward, world_up).normalized();
    const geometry::Vec3 up = geometry::cross(right, forward).normalized();

    return {
        right.x, right.y, right.z, -geometry::dot(right, position - geometry::Point3{}),
        up.x, up.y, up.z, -geometry::dot(up, position - geometry::Point3{}),
        -forward.x, -forward.y, -forward.z,
            geometry::dot(forward, position - geometry::Point3{}),
        0.0, 0.0, 0.0, 1.0,
    };
}

geometry::Mat4 Camera::projection_matrix() const {
    constexpr double vertical_fov = 0.7853981633974483;
    constexpr double near_plane = 0.1;
    constexpr double far_plane = 100.0;
    const double focal_length = 1.0 / std::tan(vertical_fov * 0.5);

    return {
        focal_length / aspect_, 0.0, 0.0, 0.0,
        0.0, focal_length, 0.0, 0.0,
        0.0, 0.0, -(far_plane + near_plane) / (far_plane - near_plane),
            -(2.0 * far_plane * near_plane) / (far_plane - near_plane),
        0.0, 0.0, -1.0, 0.0,
    };
}

void Camera::set_aspect(double aspect) noexcept {
    if (aspect > 0.0 && std::isfinite(aspect)) {
        aspect_ = aspect;
    }
}

void Camera::begin_orbit(double cursor_x, double cursor_y) noexcept {
    orbiting_ = true;
    last_cursor_x_ = cursor_x;
    last_cursor_y_ = cursor_y;
}

void Camera::orbit_to(double cursor_x, double cursor_y) noexcept {
    if (!orbiting_) {
        return;
    }
    constexpr double sensitivity = 0.005;
    yaw_ -= (cursor_x - last_cursor_x_) * sensitivity;
    pitch_ += (cursor_y - last_cursor_y_) * sensitivity;
    pitch_ = std::clamp(pitch_, -pi * 0.49, pi * 0.49);
    last_cursor_x_ = cursor_x;
    last_cursor_y_ = cursor_y;
}

void Camera::end_orbit() noexcept {
    orbiting_ = false;
}

void Camera::zoom(double scroll_offset) noexcept {
    distance_ = std::clamp(distance_ * std::exp(-scroll_offset * 0.1), 2.0, 50.0);
}

}  // namespace viewer