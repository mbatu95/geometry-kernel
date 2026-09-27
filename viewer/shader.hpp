#pragma once

#include "viewer/gl_headers.hpp"

#include <string>

namespace viewer {

class Shader {
public:
    Shader(const std::string& vertex_path, const std::string& fragment_path);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    [[nodiscard]] GLuint program() const noexcept;
    [[nodiscard]] GLint uniform_location(const char* name) const;

private:
    GLuint program_{0};
};

}  // namespace viewer