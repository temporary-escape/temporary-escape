#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Scissio {
class SCISSIO_API ShaderGrid : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, Vector3> Normal;
    typedef VertexAttribute<2, Vector2> TextureCoordinates;
    typedef VertexAttribute<3, Vector4> Tangent;
    typedef VertexAttribute<4, Matrix4> Instances;

    enum : int {
        BaseColorTexture = 0,
        NormalTexture,
        EmissiveTexture,
        MetallicRoughnessTexture,
        AmbientOcclusionTexture
    };

    explicit ShaderGrid(const Config& config);
    virtual ~ShaderGrid() = default;

    void setTransformationProjectionMatrix(const Matrix4& matrix);

    void setProjectionMatrix(const Matrix4& matrix);

    void setModelMatrix(const Matrix4& matrix);

    void bindBaseColorTexture(const Texture2D& texture);

    void bindNormalTexture(const Texture2D& texture);

    void bindEmissiveTexture(const Texture2D& texture);

    void bindMetallicRoughnessTexture(const Texture2D& texture);

    void bindAmbientOcclusionTexture(const Texture2D& texture);

private:
    int transformationProjectionMatrixUniform;
    int projectionMatrixUniform;
    int modelMatrixUniform;
};
} // namespace Scissio
