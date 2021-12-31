#include "Shader.hpp"

#include "../Utils/Exceptions.hpp"
#include "Mesh.hpp"
#include <fstream>
#include <glm/mat4x4.hpp>

#define CMP "Shader"

using namespace Scissio;

static const char* SHADER_VERSION = "#version 330 core\n";
static const char* SHADER_LINE = "#line 0 3\n";

static const char* getShaderName(const GLenum target) {
    switch (target) {
    case GL_FRAGMENT_SHADER:
        return "fragment-shader";
    case GL_VERTEX_SHADER:
        return "vertex-shader";
    case GL_GEOMETRY_SHADER:
        return "geometry-shader";
    default:
        return "unknown-shader";
    }
}

static std::string loadFile(const Path& path) {
    std::fstream file(path, std::ios::in | std::ios::ate);
    if (!file) {
        EXCEPTION("Failed to open shader file: '{}'", path.string());
    }

    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string ret;
    ret.resize(size);
    file.read(reinterpret_cast<char*>(&ret[0]), size);
    return ret;
};

Shader::Shader(const NoCreate&) : vertex(0), fragment(0), geometry(0), program() {
}

Shader::Shader(std::string name)
    : name(std::move(name)), vertex(0), fragment(0), geometry(0), program(glCreateProgram()) {
}

void Shader::compile(const std::string& source, GLuint& shader, const GLenum target) {
    shader = glCreateShader(target);

    const char* sources[4] = {SHADER_VERSION, defines.c_str(), SHADER_LINE, source.c_str()};

    glShaderSource(shader, 4, sources, nullptr);
    glCompileShader(shader);
    checkShaderStatus(shader, target);
}

void Shader::addVertexShader(const std::string& source) {
    compile(source, fragment, GL_VERTEX_SHADER);
}

void Shader::addFragmentShader(const std::string& source) {
    compile(source, vertex, GL_FRAGMENT_SHADER);
}

void Shader::addGeometryShader(const std::string& source) {
    compile(source, geometry, GL_GEOMETRY_SHADER);
}

void Shader::addVertexShader(const Path& source) {
    try {
        addVertexShader(loadFile(source));
    } catch (...) {
        EXCEPTION_NESTED("Failed to compile vertex shader from '{}'", source.string());
    }
}

void Shader::addFragmentShader(const Path& source) {
    try {
        addFragmentShader(loadFile(source));
    } catch (...) {
        EXCEPTION_NESTED("Failed to compile fragment shader from '{}'", source.string());
    }
}

void Shader::addGeometryShader(const Path& source) {
    try {
        addGeometryShader(loadFile(source));
    } catch (...) {
        EXCEPTION_NESTED("Failed to compile geometry shader from '{}'", source.string());
    }
}

void Shader::link() {
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    if (geometry > 0) {
        glAttachShader(program, geometry);
    }
    glLinkProgram(program);
    checkProgramStatus();
}

Shader::~Shader() {
    destroy();
}

Shader::Shader(Shader&& other) noexcept : vertex(0), fragment(0), geometry(0), program(0) {
    swap(other);
}

void Shader::swap(Shader& other) noexcept {
    std::swap(name, other.name);
    std::swap(vertex, other.vertex);
    std::swap(fragment, other.fragment);
    std::swap(geometry, other.geometry);
    std::swap(program, other.program);
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Shader::checkShaderStatus(const GLuint shader, const GLenum target) const {
    char infoLog[512];
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        EXCEPTION("Failed to compile {} error: {}", getShaderName(target), infoLog);
    };
}

void Shader::checkProgramStatus() const {
    char infoLog[512];
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        EXCEPTION("Failed to link shader error: {}", infoLog);
    };
}

void Shader::destroy() {
    if (program) {
        glDeleteProgram(program);
        program = 0;
    }
    if (vertex) {
        glDeleteShader(vertex);
        vertex = 0;
    }
    if (geometry) {
        glDeleteShader(geometry);
        vertex = 0;
    }
    if (fragment) {
        glDeleteShader(fragment);
        fragment = 0;
    }
}

void Shader::use() const {
    glUseProgram(program);
}

void Shader::drawArrays(const PrimitiveType type, const GLsizei count) const {
    glDrawArrays(GLenum(type), 0, count);
}

void Shader::draw(const Mesh& mesh) const {
    mesh.bind();

    if (mesh.getInstancesCount() != 0) {
        if (mesh.hasIbo()) {
            glDrawElementsInstanced(GLenum(mesh.getPrimitive()), mesh.getCount(), GLenum(mesh.getIndexType()), nullptr,
                                    mesh.getInstancesCount());
        } else {
            glDrawArraysInstanced(GLenum(mesh.getPrimitive()), 0, mesh.getCount(), mesh.getInstancesCount());
        }
    } else {
        if (mesh.hasIbo()) {
            glDrawElements(GLenum(mesh.getPrimitive()), mesh.getCount(), GLenum(mesh.getIndexType()), nullptr);
        } else {
            glDrawArrays(GLenum(mesh.getPrimitive()), 0, mesh.getCount());
        }
    }
}

GLint Shader::getUniformLocation(const std::string& location) const {
    const auto v = glGetUniformLocation(program, location.c_str());
    if (v == -1) {
        Log::w(CMP, "Shader {} uniform location {} not found", name, location);
    }
    return v;
}

GLint Shader::getUniformBlockIndex(const std::string& location) const {
    const auto v = glGetUniformBlockIndex(program, location.c_str());
    if (v == -1) {
        Log::w(CMP, "Shader {} uniform block index {} not found", name, location);
    }
    return v;
}

void Shader::uniformBlockBinding(GLuint blockIndex, GLuint blockBinding) const {
    glUniformBlockBinding(program, blockIndex, blockBinding);
}

template <> void Shader::setUniform<int>(const GLuint location, const int& value) const {
    glUniform1i(location, value);
}

template <> void Shader::setUniform<float>(const GLuint location, const float& value) const {
    glUniform1f(location, value);
}

template <> void Shader::setUniform<Vector2>(const GLuint location, const Vector2& value) const {
    glUniform2f(location, value.x, value.y);
}

template <> void Shader::setUniform<Vector2i>(const GLuint location, const Vector2i& value) const {
    glUniform2i(location, value.x, value.y);
}

template <> void Shader::setUniform<Vector3>(const GLuint location, const Vector3& value) const {
    glUniform3f(location, value.x, value.y, value.z);
}

template <> void Shader::setUniform<Vector3i>(const GLuint location, const Vector3i& value) const {
    glUniform3i(location, value.x, value.y, value.z);
}

template <> void Shader::setUniform<Vector4>(const GLuint location, const Vector4& value) const {
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

template <> void Shader::setUniform<Vector4i>(const GLuint location, const Vector4i& value) const {
    glUniform4i(location, value.x, value.y, value.z, value.w);
}

template <> void Shader::setUniform<glm::mat3x3>(const GLuint location, const glm::mat3x3& value) const {
    glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
}

template <> void Shader::setUniform<glm::mat4x4>(const GLuint location, const glm::mat4x4& value) const {
    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}
