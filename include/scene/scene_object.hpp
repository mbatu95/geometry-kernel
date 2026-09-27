#pragma once

#include "geometry/aabb.hpp"
#include "geometry/mesh.hpp"
#include "geometry/transform.hpp"

#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace scene {

class SceneObject {
public:
    SceneObject(std::string name, geometry::Mesh mesh,
                geometry::Transform transform = geometry::Transform::identity())
        : name_{std::move(name)}, mesh_{std::move(mesh)}, transform_{transform} {
        if (name_.empty()) {
            throw std::invalid_argument("SceneObject name must not be empty");
        }
    }

    [[nodiscard]] const std::string& name() const noexcept {
        return name_;
    }

    [[nodiscard]] const geometry::Mesh& mesh() const noexcept {
        return mesh_;
    }

    [[nodiscard]] const geometry::Transform& transform() const noexcept {
        return transform_;
    }

    void set_transform(const geometry::Transform& transform) noexcept {
        transform_ = transform;
    }

    [[nodiscard]] bool visible() const noexcept {
        return visible_;
    }

    void set_visible(bool visible) noexcept {
        visible_ = visible;
    }

    [[nodiscard]] std::optional<geometry::AABB> world_bounding_box() const {
        const std::optional<geometry::AABB> local_bounds = mesh_.bounding_box();
        if (!local_bounds) {
            return std::nullopt;
        }

        const geometry::Point3& minimum = local_bounds->min();
        const geometry::Point3& maximum = local_bounds->max();
        const std::array<geometry::Point3, 8> corners{
            geometry::Point3{minimum.x, minimum.y, minimum.z},
            geometry::Point3{maximum.x, minimum.y, minimum.z},
            geometry::Point3{minimum.x, maximum.y, minimum.z},
            geometry::Point3{maximum.x, maximum.y, minimum.z},
            geometry::Point3{minimum.x, minimum.y, maximum.z},
            geometry::Point3{maximum.x, minimum.y, maximum.z},
            geometry::Point3{minimum.x, maximum.y, maximum.z},
            geometry::Point3{maximum.x, maximum.y, maximum.z},
        };

        const geometry::Point3 first = transform_.apply_point(corners.front());
        geometry::AABB bounds{first, first};
        for (std::size_t index = 1; index < corners.size(); ++index) {
            bounds.expand(transform_.apply_point(corners[index]));
        }
        return bounds;
    }

private:
    std::string name_;
    geometry::Mesh mesh_;
    geometry::Transform transform_;
    bool visible_{true};
};

}  // namespace scene