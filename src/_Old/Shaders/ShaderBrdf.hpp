#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"

namespace Engine {
class ENGINE_API ShaderBrdf : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;
    typedef VertexAttribute<1, Vector2> TextureCoordinates;

    explicit ShaderBrdf(const Config& config);
    virtual ~ShaderBrdf() = default;

    ShaderBrdf(const ShaderBrdf& other) = delete;
    ShaderBrdf(ShaderBrdf&& other) = default;
    ShaderBrdf& operator=(const ShaderBrdf& other) = delete;
    ShaderBrdf& operator=(ShaderBrdf&& other) = default;
};
} // namespace Engine
