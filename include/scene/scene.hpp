#pragma once

#include "scene/scene_object.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace scene {

class Scene {
public:
    void add(SceneObject object) {
        if (find(object.name()) != nullptr) {
            throw std::invalid_argument("Scene object name must be unique: " + object.name());
        }
        objects_.push_back(std::move(object));
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return objects_.size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return objects_.empty();
    }

    [[nodiscard]] const std::vector<SceneObject>& objects() const noexcept {
        return objects_;
    }

    [[nodiscard]] auto begin() const noexcept {
        return objects_.begin();
    }

    [[nodiscard]] auto end() const noexcept {
        return objects_.end();
    }

    [[nodiscard]] SceneObject* find(const std::string& name) {
        const auto found = std::find_if(objects_.begin(), objects_.end(),
                                        [&name](const SceneObject& object) {
                                            return object.name() == name;
                                        });
        return found == objects_.end() ? nullptr : &*found;
    }

    [[nodiscard]] const SceneObject* find(const std::string& name) const {
        const auto found = std::find_if(objects_.begin(), objects_.end(),
                                        [&name](const SceneObject& object) {
                                            return object.name() == name;
                                        });
        return found == objects_.end() ? nullptr : &*found;
    }

    [[nodiscard]] bool remove(const std::string& name) {
        const auto found = std::find_if(objects_.begin(), objects_.end(),
                                        [&name](const SceneObject& object) {
                                            return object.name() == name;
                                        });
        if (found == objects_.end()) {
            return false;
        }
        objects_.erase(found);
        return true;
    }

    [[nodiscard]] std::optional<geometry::AABB> bounding_box() const {
        std::optional<geometry::AABB> bounds;
        for (const SceneObject& object : objects_) {
            if (!object.visible()) {
                continue;
            }
            const std::optional<geometry::AABB> object_bounds = object.world_bounding_box();
            if (!object_bounds) {
                continue;
            }
            if (!bounds) {
                bounds = object_bounds;
            } else {
                bounds->expand(object_bounds->min());
                bounds->expand(object_bounds->max());
            }
        }
        return bounds;
    }

private:
    std::vector<SceneObject> objects_;
};

}  // namespace scene