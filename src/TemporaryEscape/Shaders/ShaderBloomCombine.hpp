#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Graphics/VertexBuffer.hpp"
#include "../Math/Matrix.hpp"
#include "../Math/Vector.hpp"

namespace Engine {
class ShaderBloomCombine : public Shader {
public:
    using VertexCoord = VertexAttribute<0, Vector2>;
    using TexCoord = VertexAttribute<1, Vector2>;

    enum Samplers : GLuint {
        ColorTexture = 0,
        BloomTexture = 1,
    };

    explicit ShaderBloomCombine(const Config& config);

    ShaderBloomCombine(const ShaderBloomCombine& other) = delete;
    ShaderBloomCombine(ShaderBloomCombine&& other) = default;
    ShaderBloomCombine& operator=(const ShaderBloomCombine& other) = delete;
    ShaderBloomCombine& operator=(ShaderBloomCombine&& other) = default;

    void bindColorTexture(const Texture& texture) const;
    void bindBloomTexture(const Texture& texture) const;
};
} // namespace Engine
