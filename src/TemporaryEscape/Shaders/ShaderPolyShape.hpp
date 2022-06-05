#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderPolyShape : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;
    using Color = VertexAttribute<1, Vector4>;

    enum Bindings : GLuint {
        Camera = 0,
    };

    explicit ShaderPolyShape(const Config& config);
    virtual ~ShaderPolyShape() = default;

    ShaderPolyShape(const ShaderPolyShape& other) = delete;
    ShaderPolyShape(ShaderPolyShape&& other) = default;
    ShaderPolyShape& operator=(const ShaderPolyShape& other) = delete;
    ShaderPolyShape& operator=(ShaderPolyShape&& other) = default;

    void setModelMatrix(const Matrix4& matrix) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;

private:
    GLint cameraUniformIndex;
    GLint modelMatrixUniform;
};
} // namespace Engine
