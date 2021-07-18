#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderPbrBuffer : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;

    enum : int {
        DepthTexture = 0,
        BaseColorRoughnessTexture,
        NormalMetallicTexture,
        EmissiveAmbientTexture,
        BrdfTexture,
        SkyboxIrradianceTexture,
        SkyboxPrefilterTexture,
    };

    explicit ShaderPbrBuffer(const Config& config);
    virtual ~ShaderPbrBuffer() = default;

    void setViewProjectionInverseMatrix(const Matrix4& matrix);
    void bindDepthTexture(const Texture& texture);
    void bindBaseColorRoughnessTexture(const Texture& texture);
    void bindNormalMetallicTexture(const Texture& texture);
    void bindEmissiveAmbientTexture(const Texture& texture);
    void bindBrdfTexture(const Texture& texture);
    void bindSkyboxIrradianceTexture(const Texture& texture);
    void bindSkyboxPrefilterTexture(const Texture& texture);
    void setViewport(const Vector2i& viewport);
    void setEyesPos(const Vector3& pos);

private:
    int viewportUniform;
    int viewProjectionInverseMatrixUniform;
    int eyesPosUniform;
};
} // namespace Scissio
