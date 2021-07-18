#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderPbr : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, Vector3> Normal;
    typedef VertexAttribute<2, Vector2> TextureCoordinates;
    typedef VertexAttribute<3, Vector4> Tangent;

    enum : int {
        BaseColorTexture = 0,
        NormalTexture,
        EmissiveTexture,
        MetallicRoughnessTexture,
        AmbientOcclusionTexture
    };

    explicit ShaderPbr(const Config& config);
    virtual ~ShaderPbr() = 0;

    void setTransformationProjectionMatrix(const Matrix4& matrix);
    void setProjectionMatrix(const Matrix4& matrix);

    void setModelMatrix(const Matrix4& matrix);
    void setNormalMatrix(const Matrix3& matrix);

    void bindBaseColorTexture(const Texture2D& texture);
    void bindBaseColorTextureDefault();
    void bindNormalTexture(const Texture2D& texture);
    void bindNormalTextureDefault();
    void bindEmissiveTexture(const Texture2D& texture);
    void bindEmissiveTextureDefault();
    void bindMetallicRoughnessTexture(const Texture2D& texture);
    void bindMetallicRoughnessTextureDefault();
    void bindAmbientOcclusionTexture(const Texture2D& texture);
    void bindAmbientOcclusionTextureDefault();

    void setObjectId(uint16_t id);

protected:
    void complete();

    Texture2D defaultBaseColor{NO_CREATE};
    Texture2D defaultNormal{NO_CREATE};
    Texture2D defaultEmissive{NO_CREATE};
    Texture2D defaultMetallicRoughness{NO_CREATE};
    Texture2D defaultAmbientOcclusion{NO_CREATE};

    int transformationProjectionMatrixUniform;
    int projectionMatrixUniform;
    int normalMatrixUniform;
    int modelMatrixUniform;
    int objectIdUniform;
};

inline ShaderPbr::~ShaderPbr() {
}
} // namespace Scissio
