#include "viewer/renderer.hpp"

#include "viewer/gl_headers.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef GEOMETRY_VIEWER_SHADER_DIR
#error GEOMETRY_VIEWER_SHADER_DIR must be defined by CMake
#endif

namespace viewer {
namespace {

struct GPUVertex {
    float x;
    float y;
    float z;
};

std::array<float, 16> to_opengl_column_major(const geometry::Mat4& matrix) {
    std::array<float, 16> result{};
    // Mat4 is row-major; OpenGL consumes column-major arrays with GL_FALSE.
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            result[column * 4 + row] = static_cast<float>(matrix(row, column));
        }
    }
    return result;
}

}  // namespace

Renderer::GpuMesh::~GpuMesh() {
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
    }
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
    }
}

Renderer::GpuMesh::GpuMesh(GpuMesh&& other) noexcept
    : vao{std::exchange(other.vao, 0)},
      vbo{std::exchange(other.vbo, 0)},
      ebo{std::exchange(other.ebo, 0)},
      index_count{std::exchange(other.index_count, 0)} {}

Renderer::GpuMesh& Renderer::GpuMesh::operator=(GpuMesh&& other) noexcept {
    if (this != &other) {
        if (ebo != 0) {
            glDeleteBuffers(1, &ebo);
        }
        if (vbo != 0) {
            glDeleteBuffers(1, &vbo);
        }
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
        }
        vao = std::exchange(other.vao, 0);
        vbo = std::exchange(other.vbo, 0);
        ebo = std::exchange(other.ebo, 0);
        index_count = std::exchange(other.index_count, 0);
    }
    return *this;
}

Renderer::Renderer()
    : shader_{std::string{GEOMETRY_VIEWER_SHADER_DIR} + "/basic.vert",
              std::string{GEOMETRY_VIEWER_SHADER_DIR} + "/basic.frag"} {
    model_location_ = shader_.uniform_location("model");
    view_location_ = shader_.uniform_location("view");
    projection_location_ = shader_.uniform_location("projection");
    color_location_ = shader_.uniform_location("object_color");
    glEnable(GL_DEPTH_TEST);
}

std::size_t Renderer::upload_mesh(const geometry::Mesh& mesh) {
    if (mesh.triangle_count() >
        static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()) / 3) {
        throw std::length_error("Mesh has too many indices for OpenGL draw call");
    }

    std::vector<GPUVertex> vertices;
    vertices.reserve(mesh.vertex_count());
    for (const geometry::Point3& point : mesh.vertices()) {
        vertices.push_back({static_cast<float>(point.x), static_cast<float>(point.y),
                            static_cast<float>(point.z)});
    }

    std::vector<std::uint32_t> indices;
    indices.reserve(mesh.triangle_count() * 3);
    for (const geometry::TriangleIndices& triangle : mesh.triangle_indices()) {
        if (triangle.a > std::numeric_limits<std::uint32_t>::max() ||
            triangle.b > std::numeric_limits<std::uint32_t>::max() ||
            triangle.c > std::numeric_limits<std::uint32_t>::max()) {
            throw std::length_error("Mesh vertex index exceeds OpenGL index range");
        }
        indices.push_back(static_cast<std::uint32_t>(triangle.a));
        indices.push_back(static_cast<std::uint32_t>(triangle.b));
        indices.push_back(static_cast<std::uint32_t>(triangle.c));
    }

    GpuMesh gpu_mesh;
    gpu_mesh.index_count = static_cast<GLsizei>(indices.size());
    glGenVertexArrays(1, &gpu_mesh.vao);
    glGenBuffers(1, &gpu_mesh.vbo);
    glGenBuffers(1, &gpu_mesh.ebo);
    glBindVertexArray(gpu_mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gpu_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(GPUVertex)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
                 indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GPUVertex), nullptr);
    glBindVertexArray(0);
    meshes_.push_back(std::move(gpu_mesh));
    return meshes_.size() - 1;
}

std::vector<std::size_t> Renderer::upload_scene(const scene::Scene& scene) {
    std::vector<std::size_t> mesh_ids;
    mesh_ids.reserve(scene.size());
    for (const scene::SceneObject& object : scene) {
        mesh_ids.push_back(upload_mesh(object.mesh()));
    }
    return mesh_ids;
}

void Renderer::begin_frame() const {
    glClearColor(0.16F, 0.18F, 0.21F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::draw_mesh(std::size_t mesh_id, const geometry::Mat4& model,
                         const geometry::Mat4& view, const geometry::Mat4& projection,
                         const SceneRenderStyle& style) const {
    const GpuMesh& mesh = meshes_.at(mesh_id);
    const auto model_data = to_opengl_column_major(model);
    const auto view_data = to_opengl_column_major(view);
    const auto projection_data = to_opengl_column_major(projection);

    glUseProgram(shader_.program());
    glUniformMatrix4fv(model_location_, 1, GL_FALSE, model_data.data());
    glUniformMatrix4fv(view_location_, 1, GL_FALSE, view_data.data());
    glUniformMatrix4fv(projection_location_, 1, GL_FALSE, projection_data.data());
    glUniform3fv(color_location_, 1, style.color.data());

    glPolygonMode(GL_FRONT_AND_BACK, style.wireframe ? GL_LINE : GL_FILL);
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::draw_scene(const scene::Scene& scene,
                          const std::vector<std::size_t>& mesh_ids,
                          const std::vector<SceneRenderStyle>& styles,
                          const geometry::Mat4& view,
                          const geometry::Mat4& projection) const {
    if (mesh_ids.size() != scene.size() || styles.size() != scene.size()) {
        throw std::invalid_argument("Scene render data must match the Scene object count");
    }

    for (std::size_t index = 0; index < scene.size(); ++index) {
        const scene::SceneObject& object = scene.objects()[index];
        if (!object.visible()) {
            continue;
        }
        draw_mesh(mesh_ids[index], object.transform().matrix(), view, projection, styles[index]);
    }
}

}  // namespace viewer