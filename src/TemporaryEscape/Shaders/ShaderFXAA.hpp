#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderFXAA : public Shader {
public:
    using VertexCoord = VertexAttribute<0, Vector2>;
    using TexCoord = VertexAttribute<1, Vector2>;

    enum Samplers : GLuint {
        InputTexture = 0,
    };

    explicit ShaderFXAA(const Config& config);

    void bindTexture(const Texture& texture) const;
    void setFrameDirection(int value) const;
    void setFrameCount(int value) const;
    void setOutputSize(const Vector2& value) const;
    void setTextureSize(const Vector2& value) const;
    void setInputSize(const Vector2& value) const;
    void setMvpMatrix(const Matrix4& value) const;

private:
    GLint frameDirectionUniform;
    GLint frameCountUniform;
    GLint outputSizeUniform;
    GLint textureSizeUniform;
    GLint inputSizeUniform;
    GLint mvpMatrixUniform;
};
} // namespace Engine
