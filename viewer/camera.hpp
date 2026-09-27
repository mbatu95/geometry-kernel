#pragma once

#include "geometry/mat4.hpp"
#include "geometry/point3.hpp"

namespace viewer {

class Camera {
public:
    Camera() = default;

    [[nodiscard]] geometry::Mat4 view_matrix() const;
    [[nodiscard]] geometry::Mat4 projection_matrix() const;

    void set_aspect(double aspect) noexcept;
    void begin_orbit(double cursor_x, double cursor_y) noexcept;
    void orbit_to(double cursor_x, double cursor_y) noexcept;
    void end_orbit() noexcept;
    void zoom(double scroll_offset) noexcept;

private:
    geometry::Point3 target_{0.0, 0.0, 0.5};
    double yaw_{0.7853981633974483};
    double pitch_{0.48};
    double distance_{12.0};
    double aspect_{16.0 / 9.0};
    double last_cursor_x_{0.0};
    double last_cursor_y_{0.0};
    bool orbiting_{false};
};

}  // namespace viewer