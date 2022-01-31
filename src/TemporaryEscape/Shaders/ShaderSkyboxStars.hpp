#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Math/Matrix.hpp"

namespace Engine {
class ENGINE_API ShaderSkyboxStars : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, float> Brightness;
    typedef VertexAttribute<2, Color4> Color;

    explicit ShaderSkyboxStars(const Config& config);
    virtual ~ShaderSkyboxStars() = default;

    void setProjectionMatrix(const Matrix4& matrix) const {
        setUniform(projectionMatrixUniform, matrix);
    }

    void setViewMatrix(const Matrix4& matrix) const {
        setUniform(viewMatrixUniform, matrix);
    }

    void setParticleSize(const Vector2& size) const {
        setUniform(particleSizeUniform, size);
    }

private:
    int projectionMatrixUniform;
    int viewMatrixUniform;
    int particleSizeUniform;
};
} // namespace Engine
