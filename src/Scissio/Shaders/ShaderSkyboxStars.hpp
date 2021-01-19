#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderSkyboxStars : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, float> Brightness;
    typedef VertexAttribute<2, Color4> Color;

    explicit ShaderSkyboxStars(const Config& config);
    virtual ~ShaderSkyboxStars() = default;

    void setProjectionMatrix(const Matrix4& matrix) {
        setUniform(projectionMatrixUniform, matrix);
    }

    void setViewMatrix(const Matrix4& matrix) {
        setUniform(viewMatrixUniform, matrix);
    }

    void setParticleSize(const Vector2& size) {
        setUniform(particleSizeUniform, size);
    }

private:
    int projectionMatrixUniform;
    int viewMatrixUniform;
    int particleSizeUniform;
};
} // namespace Scissio
