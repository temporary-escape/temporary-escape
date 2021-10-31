#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/TextureCubemap.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderSkybox : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;

    enum : int {
        SkyboxTexture = 0,
    };

    explicit ShaderSkybox(const Config& config);
    virtual ~ShaderSkybox() = default;

    void setTransformationProjectionMatrix(const Matrix4& matrix);
    void setModelMatrix(const Matrix4& matrix);
    void bindSkyboxTexture(const TextureCubemap& texture);

private:
    int transformationProjectionMatrixUniform;
    int modelMatrixUniform;
};
} // namespace Scissio
