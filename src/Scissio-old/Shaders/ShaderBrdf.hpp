#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"

namespace Scissio {
class SCISSIO_API ShaderBrdf : public Shader {
public:
    typedef VertexAttribute<0, Vector2> Position;
    typedef VertexAttribute<1, Vector2> TextureCoordinates;

    explicit ShaderBrdf(const Config& config);
    virtual ~ShaderBrdf() = default;
};
} // namespace Scissio
