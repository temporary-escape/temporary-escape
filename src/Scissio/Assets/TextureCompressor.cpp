#include "TextureCompressor.hpp"

#include "../Utils/Exceptions.hpp"

#include <cmath>
#include <iostream>

using namespace Scissio;

static const std::string SHADER_FRAG = R"(
in vec2 v_texCoords;
out vec4 fragmentColor;
uniform sampler2D tex;
void main() {
    fragmentColor = texture(tex, v_texCoords);
}
)";

static const std::string SHADER_VERT = R"(
layout(location = 0) in vec2 position;
out vec2 v_texCoords;
void main() {
    vec2 coords = (position + 1.0) * 0.5;
    v_texCoords = vec2(coords.x, 1.0 - coords.y);
    gl_Position = vec4(position, 1.0, 1.0);
}
)";

static const float FULL_SCREEN_QUAD[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

TextureCompressor::TextureCompressor(const NoCreate&)
    : shader(NO_CREATE), fbo(NO_CREATE), fboColor(NO_CREATE), fboDepth(NO_CREATE), vao(NO_CREATE), vbo(NO_CREATE) {
}

TextureCompressor::TextureCompressor() : textureSize{0, 0}, shader("TextureCompressor"), vbo{VertexBufferType::Array} {

    shader.addFragmentShader(SHADER_FRAG);
    shader.addVertexShader(SHADER_VERT);
    shader.link();
    shader.use();
    shader.setUniform("tex", 0);

    vao.bind();
    vbo.bind();
    vbo.bufferData(FULL_SCREEN_QUAD, sizeof(FULL_SCREEN_QUAD), VertexBufferUsage::StaticDraw);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
}

Texture2D TextureCompressor::convert(Texture2D& source, const Vector2i& targetSize, const PixelType target) {
    if (targetSize.x != targetSize.y) {
        EXCEPTION("Target texture size must be a square");
    }

    const auto levels = static_cast<int>(std::log2(targetSize.x)) - 1;

    Texture2D destination{};
    destination.bind();
    destination.texParameteri(GL_TEXTURE_BASE_LEVEL, 0);
    destination.texParameteri(GL_TEXTURE_MAX_LEVEL, levels - 1);

    if (textureSize != targetSize) {
        textureSize = targetSize;

        fboDepth.setStorage(targetSize, PixelType::Depth24Stencil8);
        fbo.attach(fboDepth, FramebufferAttachment::DepthStencil);
    }

    vao.bind();
    shader.use();

    for (auto level = 0; level < levels; level++) {
        const auto w = targetSize.x >> level;

        fboColor.setStorage(level, {w, w}, PixelType::Rgba8u);
        fbo.attach(fboColor, FramebufferAttachment::Color0, level);

        glViewport(0, 0, w, w);
        source.bind();
        shader.drawArrays(PrimitiveType::Triangles, 2 * 3);
    }

    fboColor.bind();
    fboColor.texParameteri(GL_TEXTURE_BASE_LEVEL, 0);
    fboColor.texParameteri(GL_TEXTURE_MAX_LEVEL, levels - 1);

    // GLint totalBytes = 0;
    const auto internalFormat = toTextureInternalFormat(target);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Copy pixels as compressed texture
    for (auto level = 0; level < levels; level++) {
        const auto w = targetSize.x >> level;

        // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, level);
        fbo.attach(fboColor, FramebufferAttachment::Color0, level);

        destination.bind();
        glCopyTexImage2D(GL_TEXTURE_2D, level, internalFormat, 0, 0, w, w, 0);

        /*GLint compressedSize = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);
        std::cout << "Mipmap: " << level << " size: " << w << "x" << w << " bytes: " << compressedSize << std::endl;
        totalBytes += compressedSize;*/
    }

    return destination;
}
