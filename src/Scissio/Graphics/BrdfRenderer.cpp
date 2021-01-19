#include "BrdfRenderer.hpp"

#include "Framebuffer.hpp"
#include "Renderbuffer.hpp"

#include <array>

using namespace Scissio;

struct TriangleVertex {
    Vector2 position;
    Vector2 uv;
};

static const std::array<TriangleVertex, 6> VERTEX_DATA = {
    TriangleVertex{{-1.0f, -1.0f}, {0.0f, 1.0f}}, TriangleVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},
    TriangleVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},  TriangleVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},
    TriangleVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},  TriangleVertex{{1.0f, 1.0f}, {1.0f, 0.0f}}};

BrdfRenderer::BrdfRenderer(const Config& config) : config(config), shader(config) {

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(VERTEX_DATA.data(), VERTEX_DATA.size() * sizeof(TriangleVertex), VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), ShaderBrdf::Position{}, ShaderBrdf::TextureCoordinates{});
    mesh.setCount(6);
    mesh.setPrimitive(PrimitiveType::Triangles);
}

BrdfRenderer::~BrdfRenderer() = default;

Texture2D BrdfRenderer::render() {
    const auto size = Vector2i{config.brdfSize, config.brdfSize};

    Framebuffer fbo;
    Renderbuffer rbo;
    Texture2D texture;

    fbo.bind();
    glViewport(0, 0, size.x, size.y);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    glDisable(GL_BLEND);

    texture.setStorage(0, size, PixelType::Rg16f);
    fbo.attach(texture, FramebufferAttachment::Color0, 0);

    rbo.setStorage(size, PixelType::Depth24Stencil8);
    fbo.attach(rbo, FramebufferAttachment::DepthStencil);

    shader.use();
    shader.draw(mesh);

    return texture;
}
