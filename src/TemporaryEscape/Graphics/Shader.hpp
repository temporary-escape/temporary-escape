#pragma once
#include "../Math/Vector.hpp"

#include <glad/glad.h>
#include <sstream>
#include <string>

#include "../Library.hpp"
#include "../Utils/Path.hpp"
#include "Enums.hpp"
#include <fmt/format.h>
#include <variant>

namespace Engine {
class Mesh;
struct NoCreate;

class ENGINE_API Shader {
public:
    explicit Shader(const NoCreate&);
    explicit Shader(std::string name);

    virtual ~Shader();
    Shader(const Shader& other) = delete;
    Shader(Shader&& other) noexcept;
    void swap(Shader& other) noexcept;
    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(Shader&& other) noexcept;

    void addDefine(const std::string& name, const std::string& value) {
        defines += fmt::format("#define {} {}\n", name, value);
    }
    void addVertexShader(const std::string& source);
    void addFragmentShader(const std::string& source);
    void addGeometryShader(const std::string& source);
    void addVertexShader(const Path& source);
    void addFragmentShader(const Path& source);
    void addGeometryShader(const Path& source);
    void link();

    void use() const;
    void drawArrays(PrimitiveType type, GLsizei count) const;
    void draw(const Mesh& mesh) const;

    GLint getUniformLocation(const std::string& location) const;
    GLint getUniformBlockIndex(const std::string& location) const;
    void uniformBlockBinding(GLuint blockIndex, GLuint blockBinding) const;
    template <typename T> void setUniform(const std::string& location, const T& value) const {
        setUniform(getUniformLocation(location), value);
    }
    template <typename T> void setUniform(GLuint location, const T& value) const;

    GLuint getHandle() const {
        return program;
    }

private:
    void compile(const std::string& source, GLuint& shader, GLenum target);
    void checkShaderStatus(GLuint shader, const GLenum target) const;
    void checkProgramStatus() const;
    void destroy();

    // void build(const std::string& vertSource, const std::string& fragSource,
    //           const std::optional<std::string>& geomSource = std::nullopt);
    std::string name;
    std::string defines;
    GLuint vertex;
    GLuint fragment;
    GLuint geometry;
    GLuint program;
};
} // namespace Engine
