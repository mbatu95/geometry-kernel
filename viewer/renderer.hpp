#pragma once

#include "geometry/mat4.hpp"
#include "geometry/mesh.hpp"
#include "scene/scene.hpp"
#include "viewer/shader.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace viewer {

struct SceneRenderStyle {
    std::array<float, 3> color;
    bool wireframe;
};

class Renderer {
public:
    Renderer();
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] std::vector<std::size_t> upload_scene(const scene::Scene& scene);
    void begin_frame() const;
    void draw_scene(const scene::Scene& scene, const std::vector<std::size_t>& mesh_ids,
                    const std::vector<SceneRenderStyle>& styles,
                    const geometry::Mat4& view, const geometry::Mat4& projection) const;

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

    [[nodiscard]] std::size_t upload_mesh(const geometry::Mesh& mesh);
    void draw_mesh(std::size_t mesh_id, const geometry::Mat4& model,
                   const geometry::Mat4& view, const geometry::Mat4& projection,
                   const SceneRenderStyle& style) const;

    Shader shader_;
    GLint model_location_{-1};
    GLint view_location_{-1};
    GLint projection_location_{-1};
    GLint color_location_{-1};
    std::vector<GpuMesh> meshes_;
};

}  // namespace viewer