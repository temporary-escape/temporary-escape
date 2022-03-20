#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderPointCloud : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;
    using Size = VertexAttribute<1, Vector2>;
    using Color = VertexAttribute<2, Vector4>;

    enum Bindings : GLuint {
        Camera = 0,
    };

    enum Samplers : GLuint {
        PointTexture = 0,
    };

    explicit ShaderPointCloud(const Config& config);
    virtual ~ShaderPointCloud() = default;

    ShaderPointCloud(const ShaderPointCloud& other) = delete;
    ShaderPointCloud(ShaderPointCloud&& other) = default;
    ShaderPointCloud& operator=(const ShaderPointCloud& other) = delete;
    ShaderPointCloud& operator=(ShaderPointCloud&& other) = default;

    void setModelMatrix(const Matrix4& matrix) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;
    void bindTexture(const Texture& texture) const;

private:
    GLint cameraUniformIndex;
    GLint modelMatrixUniform;
};
} // namespace Engine
