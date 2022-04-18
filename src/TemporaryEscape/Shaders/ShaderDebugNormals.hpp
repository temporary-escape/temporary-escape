#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderDebugNormals : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;
    using Normal = VertexAttribute<1, Vector3>;
    using TextureCoordinates = VertexAttribute<2, Vector2>;
    using Tangent = VertexAttribute<3, Vector4>;

    enum Bindings : GLuint {
        Camera = 0,
    };

    explicit ShaderDebugNormals(const Config& config);

    ShaderDebugNormals(const ShaderDebugNormals& other) = delete;
    ShaderDebugNormals(ShaderDebugNormals&& other) = default;
    ShaderDebugNormals& operator=(const ShaderDebugNormals& other) = delete;
    ShaderDebugNormals& operator=(ShaderDebugNormals&& other) = default;

    void setModelMatrix(const Matrix4& matrix) const;
    void setNormalMatrix(const Matrix3& matrix) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;

private:
    GLint cameraUniformIndex;
    GLint normalMatrixUniform;
    GLint modelMatrixUniform;
};
} // namespace Engine
