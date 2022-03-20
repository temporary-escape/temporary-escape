#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderSkybox : public Shader {
public:
    using Position = VertexAttribute<0, Vector3>;

    enum Samplers : GLuint {
        SkyboxTexture = 0,
    };

    enum Bindings : GLuint {
        Camera = 0,
    };

    explicit ShaderSkybox(const Config& config);
    virtual ~ShaderSkybox() = default;

    ShaderSkybox(const ShaderSkybox& other) = delete;
    ShaderSkybox(ShaderSkybox&& other) = default;
    ShaderSkybox& operator=(const ShaderSkybox& other) = delete;
    ShaderSkybox& operator=(ShaderSkybox&& other) = default;

    void setModelMatrix(const Matrix4& matrix) const;
    void bindSkyboxTexture(const Texture& texture) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;

private:
    GLint cameraUniformIndex;
    GLint modelMatrixUniform;
};
} // namespace Engine
