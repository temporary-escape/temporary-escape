#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderParticleEmitter : public Shader {
public:
    enum Bindings : GLuint {
        Camera = 0,
        ParticleData = 1,
    };

    enum Samplers : GLuint {
        ParticleTexture = 0,
    };

    explicit ShaderParticleEmitter(const Config& config);

    void setTime(float value) const;
    void setModelMatrix(const Matrix4& matrix) const;
    void setModelViewMatrix(const Matrix4& matrix) const;
    void bindCameraUniform(const VertexBuffer& ubo) const;
    void bindParticleData(const VertexBuffer& ubo) const;
    void bindParticleTexture(const Texture& texture) const;

private:
    GLint cameraUniformIndex;
    GLint timeUniform;
    GLint particleDataUniformIndex;
    GLint modelMatrixUniform;
    GLint modelViewMatrixUniform;
};
} // namespace Engine
