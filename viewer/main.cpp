#include "viewer/camera.hpp"
#include "viewer/renderer.hpp"

#include "geometry/primitives.hpp"
#include "geometry/transform.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <exception>
#include <iostream>

namespace {

struct AppState {
    viewer::Camera* camera;
};

void glfw_error_callback(int, const char* description) {
    std::cerr << "GLFW error: " << description << '\n';
}

AppState* app_state(GLFWwindow* window) {
    return static_cast<AppState*>(glfwGetWindowUserPointer(window));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (height > 0) {
        app_state(window)->camera->set_aspect(static_cast<double>(width) / height);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }
    AppState* state = app_state(window);
    if (action == GLFW_PRESS) {
        double cursor_x = 0.0;
        double cursor_y = 0.0;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);
        state->camera->begin_orbit(cursor_x, cursor_y);
    } else if (action == GLFW_RELEASE) {
        state->camera->end_orbit();
    }
}

void cursor_position_callback(GLFWwindow* window, double cursor_x, double cursor_y) {
    app_state(window)->camera->orbit_to(cursor_x, cursor_y);
}

void scroll_callback(GLFWwindow* window, double, double scroll_y) {
    app_state(window)->camera->zoom(scroll_y);
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

}  // namespace

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Unable to initialize GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1100, 760, "Geometry Kernel Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Unable to create an OpenGL 3.3 Core window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    int exit_code = 0;
    try {
        viewer::Camera camera;
        AppState state{&camera};
        glfwSetWindowUserPointer(window, &state);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window, key_callback);

        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        framebuffer_size_callback(window, framebuffer_width, framebuffer_height);

        {
            viewer::Renderer renderer;
            const geometry::Mesh ground = geometry::make_xy_grid(10.0, 10.0, 10, 10);
            const geometry::Mesh box = geometry::make_box(1.0, 1.0, 1.0).transformed(
                geometry::Transform::translation(0.0, 0.0, 0.5));
            const std::size_t ground_id = renderer.upload(ground);
            const std::size_t box_id = renderer.upload(box);
            const geometry::Mat4 identity = geometry::Mat4::identity();

            while (glfwWindowShouldClose(window) == GLFW_FALSE) {
                renderer.begin_frame();
                renderer.draw(ground_id, identity, camera.view_matrix(),
                              camera.projection_matrix(), {0.55F, 0.62F, 0.68F}, true);
                renderer.draw(box_id, identity, camera.view_matrix(),
                              camera.projection_matrix(), {0.86F, 0.43F, 0.24F}, false);
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Viewer error: " << error.what() << '\n';
        exit_code = 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return exit_code;
}