#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Math/Matrix.hpp"

namespace Scissio {
class SCISSIO_API ShaderWireframe : public Shader {
public:
    typedef VertexAttribute<0, Vector3> Position;

    explicit ShaderWireframe(const Config& config);
    virtual ~ShaderWireframe() = default;

    void setTransformationProjectionMatrix(const Matrix4& matrix);
    void setModelMatrix(const Matrix4& matrix);
    void setEyesPos(const Vector3& pos);
    void setColor(const Color4& color);

private:
    int transformationProjectionMatrixUniform;
    int modelMatrixUniform;
    int colorUniform;
    int eyesPosUniform;
};
} // namespace Scissio
