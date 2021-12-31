#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderSkyboxNebula : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;

    explicit ShaderSkyboxNebula(const Config& config);
    virtual ~ShaderSkyboxNebula() = default;

    void setProjectionMatrix(const Matrix4& matrix) const {
        setUniform(projectionMatrixUniform, matrix);
    }

    void setViewMatrix(const Matrix4& matrix) const {
        setUniform(viewMatrixUniform, matrix);
    }

    void setColor(const Color4& color) const {
        setUniform(colorUniform, color);
    }

    void setOffset(const Vector3& offset) const {
        setUniform(offsetUniform, offset);
    }

    void setScale(const float scale) const {
        setUniform(scaleUniform, scale);
    }

    void setIntensity(const float intensity) const {
        setUniform(intensityUniform, intensity);
    }

    void setFalloff(const float falloff) const {
        setUniform(falloffUniform, falloff);
    }

private:
    int projectionMatrixUniform;
    int viewMatrixUniform;
    int colorUniform;
    int offsetUniform;
    int scaleUniform;
    int intensityUniform;
    int falloffUniform;
};
} // namespace Scissio
