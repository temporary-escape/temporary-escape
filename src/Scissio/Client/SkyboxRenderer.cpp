#include "SkyboxRenderer.hpp"
#include <array>
#include <cmath>

using namespace Scissio;

static const std::array<Matrix4, 6> CAPTURE_VIEWS = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
};

// Simple skybox box with two triangles per side.
static const float SKYBOX_VERTICES[] = {
    // positions
    -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
    -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

    1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
    1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,
};

static const std::array<GLenum, 6> CUBEMAP_ENUMS = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

static const float PI = static_cast<float>(std::atan(1) * 4);

struct StarVertex {
    Vector3 position;
    float brightness;
    Color4 color;
};

// The projection matrix that will be used to generate the skybox.
// This must be 90 degrees view (PI/2)
static const glm::mat4 CAPTURE_PROJECTION = glm::perspective(PI / 2.0, 1.0, 0.1, 1000.0);

SkyboxRenderer::SkyboxRenderer(const Config& config)
    : config(config), shaderStars(config), shaderNebula(config), shaderIrradiance(config), shaderPrefilter(config) {
    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(SKYBOX_VERTICES, sizeof(SKYBOX_VERTICES), VertexBufferUsage::StaticDraw);

    box.addVertexBuffer(std::move(vbo), ShaderSkyboxNebula::Position{});
    box.setCount(6 * 2 * 3);
    box.setPrimitive(PrimitiveType::Triangles);

    fboDepth.setStorage({512, 512}, PixelType::Depth24Stencil8);
    fbo.attach(fboDepth, FramebufferAttachment::DepthStencil);
}

TextureCubemap SkyboxRenderer::render(const uint64_t seed) {
    const auto size = config.skyboxSize;
    std::mt19937_64 rng(seed); // 741852

    TextureCubemap result;
    result.bind();

    for (const auto& side : CUBEMAP_ENUMS) {
        glTexImage2D(side, 0, GL_RGB8, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    result.texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    result.texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    result.texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_BASE_LEVEL, 0);
    result.texParameteri(GL_TEXTURE_MAX_LEVEL, 0);

    fbo.bind();

    fboDepth.setStorage({size, size}, PixelType::Depth24Stencil8);
    fbo.attach(fboDepth, FramebufferAttachment::DepthStencil);

    static const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);
    glViewport(0, 0, size, size);
    glDisable(GL_DEPTH_TEST);

    for (const auto& side : CUBEMAP_ENUMS) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, side, result.getHandle(), 0);
        static Color4 black{0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, &black.x);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    renderStars(rng, result, 20000, 0.05f);
    renderStars(rng, result, 200, 0.13f);
    renderNebulas(rng, result);

    Framebuffer::DefaultFramebuffer.bind();

    fboDepth = Renderbuffer{NO_CREATE}; // Destroy

    return result;
}

void SkyboxRenderer::renderStars(std::mt19937_64& rng, TextureCubemap& result, const int count,
                                 const float particleSize) {
    std::uniform_real_distribution<float> distPosition(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distColor(0.9f, 1.0f);
    std::uniform_real_distribution<float> distBrightness(0.5f, 1.0f);

    std::vector<StarVertex> stars;

    stars.resize(count);

    for (size_t i = 0; i < stars.size(); i++) {
        auto& star = stars[i];
        star.position = glm::normalize(Vector3{distPosition(rng), distPosition(rng), distPosition(rng)}) * 100.0f;
        star.color = Color4{distColor(rng), distColor(rng), distColor(rng), 1.0f};
        star.brightness = distBrightness(rng);
    }

    VertexArray vao{};
    vao.bind();

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(stars.data(), stars.size() * sizeof(StarVertex), VertexBufferUsage::StaticDraw);

    Mesh mesh;
    mesh.setPrimitive(PrimitiveType::Points);
    mesh.setCount(static_cast<int>(stars.size()));
    mesh.addVertexBuffer(std::move(vbo), ShaderSkyboxStars::Position{}, ShaderSkyboxStars::Brightness{},
                         ShaderSkyboxStars::Color{});
    shaderStars.use();
    shaderStars.setProjectionMatrix(CAPTURE_PROJECTION);
    shaderStars.setParticleSize(Vector2{particleSize, particleSize});

    for (unsigned int i = 0; i < 6; ++i) {
        shaderStars.setViewMatrix(CAPTURE_VIEWS[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CUBEMAP_ENUMS[i], result.getHandle(), 0);
        shaderStars.draw(mesh);
    }
}

void SkyboxRenderer::renderNebulas(std::mt19937_64& rng, TextureCubemap& result) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    shaderNebula.use();
    shaderNebula.setProjectionMatrix(CAPTURE_PROJECTION);

    while (true) {
        shaderNebula.setScale(dist(rng) * 0.5f + 0.25f);
        shaderNebula.setIntensity(dist(rng) * 0.2f + 0.9f);
        shaderNebula.setColor(Color4{dist(rng), dist(rng), dist(rng), 1.0f});
        shaderNebula.setFalloff(dist(rng) * 3.0f + 3.0f);
        shaderNebula.setOffset(
            Vector3{dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f, dist(rng) * 2000.0f - 1000.0f});

        for (unsigned int i = 0; i < 6; ++i) {
            shaderNebula.setViewMatrix(CAPTURE_VIEWS[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CUBEMAP_ENUMS[i], result.getHandle(), 0);
            shaderNebula.draw(box);
        }

        if (dist(rng) < 0.5f) {
            break;
        }
    }
}

TextureCubemap SkyboxRenderer::irradiance(const TextureCubemap& texture) {
    // Source:
    // https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/6.pbr/2.2.2.ibl_specular_textured/ibl_specular_textured.cpp

    const auto size = Vector2i{config.skyboxIrradianceSize};

    TextureCubemap result;
    result.setStorage(0, size, PixelType::Rgb16f);
    result.texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    shaderIrradiance.use();
    shaderIrradiance.setProjectionMatrix(CAPTURE_PROJECTION);
    shaderIrradiance.bindSkyboxTexture(texture);

    fbo.bind();

    fboDepth.setStorage(size, PixelType::Depth24Stencil8);
    fbo.attach(fboDepth, FramebufferAttachment::DepthStencil);

    glViewport(0, 0, size.x, size.y);

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    for (unsigned int i = 0; i < 6; ++i) {
        shaderIrradiance.setViewMatrix(CAPTURE_VIEWS[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CUBEMAP_ENUMS[i], result.getHandle(), 0);
        shaderNebula.draw(box);
    }

    result.generateMipmaps();

    fboDepth = Renderbuffer{NO_CREATE}; // Destroy

    return result;
}

TextureCubemap SkyboxRenderer::prefilter(const TextureCubemap& texture) {
    // Source:
    // https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/6.pbr/2.2.2.ibl_specular_textured/ibl_specular_textured.cpp

    const auto size = Vector2i{config.skyboxPrefilterSize};
    const auto mipmaps = static_cast<int>(std::log2(size.x));

    TextureCubemap result;
    result.setStorage(0, size, PixelType::Rgb16f);
    result.generateMipmaps();
    result.texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    result.texParameteri(GL_TEXTURE_BASE_LEVEL, 0);
    result.texParameteri(GL_TEXTURE_MAX_LEVEL, mipmaps - 1);

    shaderPrefilter.use();
    shaderPrefilter.setProjectionMatrix(CAPTURE_PROJECTION);
    shaderPrefilter.bindSkyboxTexture(texture);

    fbo.bind();

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    for (auto mip = 0; mip < mipmaps; ++mip) {
        const auto width = static_cast<int>(size.x * std::pow(0.5, mip));

        glViewport(0, 0, width, width);
        const auto roughness = static_cast<float>(mip) / static_cast<float>(mipmaps - 1);

        fboDepth.setStorage({width, width}, PixelType::Depth24Stencil8);
        fbo.attach(fboDepth, FramebufferAttachment::DepthStencil);

        shaderPrefilter.setRoughness(roughness);

        for (unsigned int i = 0; i < 6; ++i) {
            shaderPrefilter.setViewMatrix(CAPTURE_VIEWS[i]);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CUBEMAP_ENUMS[i], result.getHandle(), mip);

            static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
            glClearBufferfv(GL_COLOR, 0, &black.x);
            glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

            shaderPrefilter.draw(box);
        }
    }

    fboDepth = Renderbuffer{NO_CREATE}; // Destroy

    return result;
}
