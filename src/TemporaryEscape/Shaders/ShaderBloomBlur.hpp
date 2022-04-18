#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderBloomBlur : public Shader {
public:
    using VertexCoord = VertexAttribute<0, Vector2>;
    using TexCoord = VertexAttribute<1, Vector2>;

    enum Samplers : GLuint {
        InputTexture = 0,
    };

    explicit ShaderBloomBlur(const Config& config);

    ShaderBloomBlur(const ShaderBloomBlur& other) = delete;
    ShaderBloomBlur(ShaderBloomBlur&& other) = default;
    ShaderBloomBlur& operator=(const ShaderBloomBlur& other) = delete;
    ShaderBloomBlur& operator=(ShaderBloomBlur&& other) = default;

    void bindTexture(const Texture& texture) const;
    void setHorizontal(bool value) const;
    void setSize(int value) const;

private:
    GLint horizontalUniform;
    GLint weightsUniform;
    GLint radiusUniform;
};
} // namespace Engine
