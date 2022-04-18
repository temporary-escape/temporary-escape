#pragma once
#include "../Graphics/Framebuffer.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/PixelType.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include <list>
#include <memory>

namespace Engine {
class Texture2D;

class ENGINE_API TextureCompressor {
public:
    explicit TextureCompressor(const NoCreate&);
    TextureCompressor();
    virtual ~TextureCompressor() = default;

    Texture2D convert(Texture2D& source, const Vector2i& targetSize, PixelType target);

private:
    Shader shader;
    Framebuffer fbo;
    Texture2D fboColor;
    VertexArray vao;
    VertexBuffer vbo;
};
} // namespace Engine
