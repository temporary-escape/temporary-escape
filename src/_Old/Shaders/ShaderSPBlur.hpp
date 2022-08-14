#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture.hpp"
#include "../Math/Matrix.hpp"

namespace Engine {
class ENGINE_API ShaderSPBlur : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;
    typedef VertexAttribute<1, Vector2> TextureCoordinates;

    enum Samplers : int {
        ColorTexture = 0,
    };

    explicit ShaderSPBlur(const Config& config);
    virtual ~ShaderSPBlur() = default;

    ShaderSPBlur(const ShaderSPBlur& other) = delete;
    ShaderSPBlur(ShaderSPBlur&& other) = default;
    ShaderSPBlur& operator=(const ShaderSPBlur& other) = delete;
    ShaderSPBlur& operator=(ShaderSPBlur&& other) = default;

    void bindColorTexture(const Texture& texture) const;
};
} // namespace Engine
