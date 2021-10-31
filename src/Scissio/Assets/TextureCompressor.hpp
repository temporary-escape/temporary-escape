#pragma once
#include "../Graphics/Framebuffer.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/PixelType.hpp"
#include "../Graphics/Shader.hpp"
#include "../Graphics/Texture2D.hpp"
#include <list>
#include <memory>

namespace Scissio {
class Texture2D;

struct CompressedTextureChunk {
    int level;
    int width;
    std::unique_ptr<uint8_t[]> pixels;
};

class SCISSIO_API TextureCompressor {
public:
    explicit TextureCompressor();
    virtual ~TextureCompressor() = default;

    Texture2D convert(Texture2D& source, const Vector2i& targetSize, PixelType target);

private:
    Vector2i textureSize;
    Shader shader;
    Framebuffer fbo;
    Renderbuffer fboDepth;
    Texture2D fboColor;
    VertexArray vao;
    VertexBuffer vbo;
};
} // namespace Scissio
