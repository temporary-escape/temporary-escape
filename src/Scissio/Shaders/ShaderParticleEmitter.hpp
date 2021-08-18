#pragma once

#include "../Config.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderParticleEmitter : public Shader {
public:
    enum : int {
        ParticleTexture,
    };

    explicit ShaderParticleEmitter(const Config& config);
    virtual ~ShaderParticleEmitter() = default;

    void setViewMatrix(const Matrix4& matrix);
    void setProjectionMatrix(const Matrix4& matrix);
    void setModelMatrix(const Matrix4& matrix);
    void bindParticleTexture(const Texture2D& texture);
    void setPosition(const Vector3& vector);

private:
    int viewMatrixUniform;
    int projectionMatrixUniform;
    int modelMatrixUniform;
    int positionUniform;
};
} // namespace Scissio
