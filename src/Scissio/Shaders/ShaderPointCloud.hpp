#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderPointCloud : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, float> Size;
    typedef VertexAttribute<2, Vector4> Color;

    enum : int { PointTexture = 0 };

    explicit ShaderPointCloud(const Config& config);
    virtual ~ShaderPointCloud() = default;

    void setViewMatrix(const Matrix4& matrix);
    void setProjectionMatrix(const Matrix4& matrix);
    void setModelMatrix(const Matrix4& matrix);

    void bindPointTexture(const Texture2D& texture);

private:
    int viewMatrixUniform;
    int projectionMatrixUniform;
    int modelMatrixUniform;
};
} // namespace Scissio
