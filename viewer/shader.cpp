#include "viewer/shader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace viewer {
namespace {

std::string read_file(const std::string& path) {
    std::ifstream file{path};
    if (!file) {
        throw std::runtime_error("Unable to open shader file: " + path);
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint compile_shader(GLenum type, const std::string& source, const std::string& path) {
    const GLuint shader = glCreateShader(type);
    const char* source_pointer = source.c_str();
    glShaderSource(shader, 1, &source_pointer, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::string log(static_cast<std::size_t>(log_length), '\0');
        glGetShaderInfoLog(shader, log_length, nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compilation failed for " + path + ":\n" + log);
    }
    return shader;
}

}  // namespace

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) {
    const GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, read_file(vertex_path), vertex_path);
    GLuint fragment_shader = 0;
    try {
        fragment_shader = compile_shader(
            GL_FRAGMENT_SHADER, read_file(fragment_path), fragment_path);
        program_ = glCreateProgram();
        glAttachShader(program_, vertex_shader);
        glAttachShader(program_, fragment_shader);
        glLinkProgram(program_);

        GLint linked = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE) {
            GLint log_length = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);
            std::string log(static_cast<std::size_t>(log_length), '\0');
            glGetProgramInfoLog(program_, log_length, nullptr, log.data());
            glDeleteProgram(program_);
            program_ = 0;
            throw std::runtime_error("Shader program link failed:\n" + log);
        }
    } catch (...) {
        glDeleteShader(vertex_shader);
        if (fragment_shader != 0) {
            glDeleteShader(fragment_shader);
        }
        throw;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Shader::~Shader() {
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
}

GLuint Shader::program() const noexcept {
    return program_;
}

GLint Shader::uniform_location(const char* name) const {
    const GLint location = glGetUniformLocation(program_, name);
    if (location < 0) {
        throw std::runtime_error(std::string{"Required shader uniform not found: "} + name);
    }
    return location;
}

}  // namespace viewer