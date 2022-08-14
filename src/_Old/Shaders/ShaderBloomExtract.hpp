#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderBloomExtract : public Shader {
public:
    using VertexCoord = VertexAttribute<0, Vector2>;
    using TexCoord = VertexAttribute<1, Vector2>;

    enum Samplers : GLuint {
        InputTexture = 0,
    };

    explicit ShaderBloomExtract(const Config& config);

    ShaderBloomExtract(const ShaderBloomExtract& other) = delete;
    ShaderBloomExtract(ShaderBloomExtract&& other) = default;
    ShaderBloomExtract& operator=(const ShaderBloomExtract& other) = delete;
    ShaderBloomExtract& operator=(ShaderBloomExtract&& other) = default;

    void bindTexture(const Texture& texture) const;
    void setBrightness(float value) const;

private:
    GLint brightnessUniform;
};
} // namespace Engine
