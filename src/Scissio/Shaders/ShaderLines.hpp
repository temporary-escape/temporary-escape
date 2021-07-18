#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderLines : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, float> Dummy;
    typedef VertexAttribute<2, Color4> Color;

    explicit ShaderLines(const Config& config);
    virtual ~ShaderLines() = default;

    void setProjectionViewMatrix(const Matrix4& matrix);
    void setModelMatrix(const Matrix4& matrix);

private:
    int projectionViewMatrixUniform;
    int modelMatrixUniform;
};
} // namespace Scissio
