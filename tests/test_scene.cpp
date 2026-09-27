#include "geometry/primitives.hpp"
#include "scene/scene.hpp"

#include <cmath>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string_view>

namespace {

int failures = 0;

void check(bool condition, std::string_view description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

bool near(double actual, double expected) {
    return std::abs(actual - expected) <= 1e-9;
}

bool near(const geometry::Point3& actual, const geometry::Point3& expected) {
    return near(actual.x, expected.x) && near(actual.y, expected.y) &&
           near(actual.z, expected.z);
}

bool same_bounds(const geometry::AABB& actual, const geometry::Point3& minimum,
                 const geometry::Point3& maximum) {
    return near(actual.min(), minimum) && near(actual.max(), maximum);
}

void test_scene_object() {
    using namespace geometry;
    using namespace scene;

    const Mesh mesh = make_box(2.0, 2.0, 2.0);
    SceneObject object{"piece", mesh};
    check(object.name() == "piece", "SceneObject name");
    check(object.mesh().vertex_count() == mesh.vertex_count(), "SceneObject Mesh access");
    check(object.transform().matrix() == Mat4::identity(), "SceneObject defaults to identity");
    check(object.visible(), "SceneObject is visible by default");

    object.set_transform(Transform::translation(1.0, 2.0, 3.0));
    check(near(object.transform().apply_point(Point3{}), Point3{1.0, 2.0, 3.0}),
          "SceneObject transform can be changed");
    object.set_visible(false);
    check(!object.visible(), "SceneObject visibility can be changed");

    bool empty_name_threw = false;
    try {
        static_cast<void>(SceneObject{"", mesh});
    } catch (const std::invalid_argument&) {
        empty_name_threw = true;
    }
    check(empty_name_threw, "SceneObject rejects empty name");
}

void test_scene_collection() {
    using namespace geometry;
    using namespace scene;

    Scene world;
    check(world.empty() && world.size() == 0, "New Scene is empty");
    world.add(SceneObject{"board", make_xy_plane(4.0, 4.0)});
    world.add(SceneObject{"piece", make_box(1.0, 1.0, 1.0)});
    check(!world.empty() && world.size() == 2, "Scene add and size");
    check(world.find("board") != nullptr, "Scene finds existing object");
    check(world.find("missing") == nullptr, "Scene returns null for missing object");

    const Scene& const_world = world;
    check(const_world.find("piece") != nullptr, "Const Scene lookup");
    check(world.objects().size() == 2, "Scene exposes read-only object collection");
    std::size_t iterated = 0;
    for (const SceneObject& object : world) {
        static_cast<void>(object);
        ++iterated;
    }
    check(iterated == 2, "Scene supports read-only iteration");

    bool duplicate_name_threw = false;
    try {
        world.add(SceneObject{"board", make_box(1.0, 1.0, 1.0)});
    } catch (const std::invalid_argument&) {
        duplicate_name_threw = true;
    }
    check(duplicate_name_threw, "Scene rejects duplicate name");
    check(world.remove("piece"), "Scene removes existing object");
    check(!world.remove("piece"), "Scene remove returns false for missing object");
    check(world.size() == 1, "Scene size after removal");
}

void test_world_bounds() {
    using namespace geometry;
    using namespace scene;

    const Mesh box = make_box(2.0, 4.0, 6.0);
    const SceneObject translated{
        "translated", box, Transform::translation(3.0, -2.0, 1.0)};
    const auto translated_bounds = translated.world_bounding_box();
    check(translated_bounds &&
              same_bounds(*translated_bounds, Point3{2.0, -4.0, -2.0},
                          Point3{4.0, 0.0, 4.0}),
          "World bounds include translation");

    const SceneObject scaled{"scaled", box, Transform::scaling(2.0, 0.5, 3.0)};
    const auto scaled_bounds = scaled.world_bounding_box();
    check(scaled_bounds &&
              same_bounds(*scaled_bounds, Point3{-2.0, -1.0, -9.0},
                          Point3{2.0, 1.0, 9.0}),
          "World bounds include scale");

    const SceneObject rotated{
        "rotated", make_box(2.0, 1.0, 1.0),
        Transform::rotation_z(std::numbers::pi / 2.0)};
    const auto rotated_bounds = rotated.world_bounding_box();
    check(rotated_bounds &&
              same_bounds(*rotated_bounds, Point3{-0.5, -1.0, -0.5},
                          Point3{0.5, 1.0, 0.5}),
          "World bounds include rotation of all box corners");
}

void test_scene_bounds() {
    using namespace geometry;
    using namespace scene;

    Scene world;
    world.add(SceneObject{
        "left", make_box(2.0, 2.0, 2.0), Transform::translation(-3.0, 0.0, 0.0)});
    world.add(SceneObject{
        "right", make_box(2.0, 2.0, 2.0), Transform::translation(3.0, 0.0, 0.0)});
    const auto bounds = world.bounding_box();
    check(bounds && same_bounds(*bounds, Point3{-4.0, -1.0, -1.0},
                                Point3{4.0, 1.0, 1.0}),
          "Scene bounds contain multiple objects");

    world.find("right")->set_visible(false);
    const auto visible_bounds = world.bounding_box();
    check(visible_bounds &&
              same_bounds(*visible_bounds, Point3{-4.0, -1.0, -1.0},
                          Point3{-2.0, 1.0, 1.0}),
          "Invisible objects do not affect Scene bounds");

    const Scene empty;
    check(!empty.bounding_box(), "Empty Scene has no bounding box");

    Scene no_visible_geometry;
    no_visible_geometry.add(SceneObject{"hidden", make_box(1.0, 1.0, 1.0)});
    no_visible_geometry.find("hidden")->set_visible(false);
    check(!no_visible_geometry.bounding_box(), "Scene with no visible geometry has no bounds");
}

}  // namespace

int main() {
    test_scene_object();
    test_scene_collection();
    test_world_bounds();
    test_scene_bounds();
    return failures == 0 ? 0 : 1;
}