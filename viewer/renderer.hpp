#pragma once

#include "geometry/mat4.hpp"
#include "geometry/mesh.hpp"
#include "viewer/shader.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace viewer {

class Renderer {
public:
    Renderer();
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] std::size_t upload(const geometry::Mesh& mesh);
    void begin_frame() const;
    void draw(std::size_t mesh_id, const geometry::Mat4& model,
              const geometry::Mat4& view, const geometry::Mat4& projection,
              const std::array<float, 3>& color, bool wireframe) const;

private:
    struct GpuMesh {
        GLuint vao{0};
        GLuint vbo{0};
        GLuint ebo{0};
        GLsizei index_count{0};

        GpuMesh() = default;
        ~GpuMesh();
        GpuMesh(const GpuMesh&) = delete;
        GpuMesh& operator=(const GpuMesh&) = delete;
        GpuMesh(GpuMesh&& other) noexcept;
        GpuMesh& operator=(GpuMesh&& other) noexcept;
    };

    Shader shader_;
    GLint model_location_{-1};
    GLint view_location_{-1};
    GLint projection_location_{-1};
    GLint color_location_{-1};
    std::vector<GpuMesh> meshes_;
};

}  // namespace viewer