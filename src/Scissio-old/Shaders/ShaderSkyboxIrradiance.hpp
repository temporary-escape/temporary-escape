#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/TextureCubemap.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderSkyboxIrradiance : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;

    enum : int {
        SkyboxTexture = 0,
    };

    explicit ShaderSkyboxIrradiance(const Config& config);
    virtual ~ShaderSkyboxIrradiance() = default;

    void setProjectionMatrix(const Matrix4& matrix);
    void setViewMatrix(const Matrix4& matrix);
    void bindSkyboxTexture(const TextureCubemap& texture);

private:
    int projectionMatrixUniform;
    int viewMatrixUniform;
};
} // namespace Scissio
