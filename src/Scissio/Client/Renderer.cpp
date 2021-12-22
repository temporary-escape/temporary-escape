#include "Renderer.hpp"

#define CMP "Renderer"

// Simple skybox box with two triangles per side.
static const float skyboxVertices[] = {
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

using namespace Scissio;

Renderer::Renderer(Canvas2D& canvas, const Config& config, AssetManager& assetManager, Client& client)
    : canvas(canvas), gui(canvas, config, assetManager), client(client), skyboxRenderer(config),
      widgetDebugStats(gui, client.getStats()), shaders{ShaderSkybox{config}, ShaderModel{config}},
      cameraUbo(VertexBufferType::Uniform) {

    const auto setColor = [](Texture2D& texture, const Color4& color) {
        std::unique_ptr<uint8_t[]> pixels(new uint8_t[8 * 8 * 4]);
        for (size_t i = 0; i < 8 * 8 * 4; i += 4) {
            pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
            pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
            pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
            pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
        }

        texture.setStorage(0, {8, 8}, PixelType::Rgba8u);
        texture.setPixels(0, {0, 0}, {8, 8}, PixelType::Rgba8u, pixels.get());
    };

    setColor(defaultBaseColorTexture, Color4{1.0f, 0.0f, 1.0f, 1.0f});
    setColor(defaultNormalTexture, Color4{0.5f, 0.5f, 1.0f, 1.0f});
    setColor(defaultEmissiveTexture, Color4{0.0f, 0.0f, 0.0f, 1.0f});
    setColor(defaultMetallicRoughnessTexture, Color4{0.0f, 0.5f, 0.5f, 1.0f});
    setColor(defaultAmbientOcclusionTexture, Color4{1.0f, 1.0f, 1.0f, 1.0f});
    setColor(defaultSkyboxTexture, Color4{0.0f, 0.0f, 0.0f, 0.0f});

    camera.lookAt({2.0f, 2.0f, 2.0f}, {0.0f, 0.0f, 0.0f});
}

Renderer::~Renderer() = default;

void Renderer::createSkybox(uint64_t seed) {
    Log::i(CMP, "Creating skybox for seed {}", seed);

    if (!skybox.mesh) {
        VertexBuffer vbo(VertexBufferType::Array);
        vbo.bufferData(skyboxVertices, sizeof(skyboxVertices), VertexBufferUsage::StaticDraw);

        skybox.mesh.addVertexBuffer(std::move(vbo), ShaderSkybox::Position{});
        skybox.mesh.setCount(6 * 2 * 3);
        skybox.mesh.setPrimitive(PrimitiveType::Triangles);
    }

    skybox.seed = seed;

    skybox.textures = skyboxRenderer.renderAndFilter(seed);
}

void Renderer::render(const Vector2i& viewport) {
    const auto t0 = std::chrono::steady_clock::now();

    camera.setProjection(viewport, 70.0f);

    CameraUniform cameraUniform{
        camera.getProjectionMatrix() * camera.getViewMatrix(),
    };
    cameraUbo.bufferData(&cameraUniform, sizeof(CameraUniform), VertexBufferUsage::StaticDraw);

    if (const auto scene = client.getScene(); scene != nullptr) {
        if (gBuffer.size != viewport) {
            gBuffer.fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
            gBuffer.fboColorRoughness.setStorage(0, viewport, PixelType::Rgba8u);
            gBuffer.fboEmissiveMetallic.setStorage(0, viewport, PixelType::Rgba8u);
            gBuffer.fboNormalAmbient.setStorage(0, viewport, PixelType::Rgba16f);
            gBuffer.fboObjectId.setStorage(0, viewport, PixelType::Rg8u);

            if (!gBuffer.fboInit) {
                gBuffer.fboInit = true;
                gBuffer.fbo.attach(gBuffer.fboColorRoughness, FramebufferAttachment::Color0, 0);
                gBuffer.fbo.attach(gBuffer.fboEmissiveMetallic, FramebufferAttachment::Color1, 0);
                gBuffer.fbo.attach(gBuffer.fboNormalAmbient, FramebufferAttachment::Color2, 0);
                gBuffer.fbo.attach(gBuffer.fboObjectId, FramebufferAttachment::Color3, 0);
                gBuffer.fbo.attach(gBuffer.fboDepth, FramebufferAttachment::DepthStencil, 0);
            }
        }

        renderSceneSkybox(viewport, *scene);

        gBuffer.fbo.bind();
        renderScenePbr(viewport, *scene);
        Framebuffer::DefaultFramebuffer.bind();

        // blit(viewport, gBuffer.fbo, Framebuffer::DefaultFramebuffer, FramebufferAttachment::Color2);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    canvas.beginFrame(viewport);
    gui.reset();

    widgetDebugStats.render();

    gui.render(viewport);
    canvas.endFrame();

    const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    client.getStats().render.frameTimeMs.store(tDiff.count());
}

void Renderer::blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target,
                    const FramebufferAttachment attachment) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.getHandle());
    glReadBuffer(GLuint(attachment));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getHandle());
    glViewport(0, 0, viewport.x, viewport.y);
    glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Renderer::renderSceneSkybox(const Vector2i& viewport, Scene& scene) {
    auto& componentSystemSkybox = scene.getComponentSystem<ComponentSkybox>();

    auto componentSkybox = componentSystemSkybox.begin();
    if (componentSkybox != componentSystemSkybox.end()) {
        auto& component = *componentSkybox;
        if (skybox.seed != component->getSeed()) {
            createSkybox(component->getSeed());
        }
    }

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    shaders.skybox.use();
    shaders.skybox.bindCameraUniform(cameraUbo);

    if (componentSkybox != componentSystemSkybox.end()) {
        renderComponent(**componentSkybox);
    }
}

void Renderer::renderSceneForward(const Vector2i& viewport, Scene& scene) {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::renderScenePbr(const Vector2i& viewport, Scene& scene) {
    auto& componentSystemModel = scene.getComponentSystem<ComponentModel>();

    GLuint attachments[4] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
    };
    glDrawBuffers(4, attachments);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);

    glViewport(0, 0, viewport.x, viewport.y);
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfv(GL_COLOR, 3, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    shaders.model.use();
    shaders.model.bindCameraUniform(cameraUbo);

    for (auto& component : componentSystemModel) {
        renderComponent(*component);
    }
}

void Renderer::renderComponent(ComponentSkybox& component) {
    shaders.skybox.setModelMatrix(component.getObject().getTransform());
    shaders.skybox.bindSkyboxTexture(skybox.textures.texture);
    shaders.skybox.draw(skybox.mesh);
}

void Renderer::renderComponent(ComponentModel& component) {
    const auto& transform = component.getObject().getTransform();
    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));
    shaders.model.setModelMatrix(transform);
    shaders.model.setNormalMatrix(transformInverted);

    for (const auto& primitive : component.getModel()->getPrimitives()) {
        shaders.model.bindMaterialUniform(primitive.ubo);

        if (primitive.material.baseColorTexture) {
            shaders.model.bindBaseColorTexture(primitive.material.baseColorTexture->getTexture());
        } else {
            shaders.model.bindBaseColorTexture(defaultBaseColorTexture);
        }

        if (primitive.material.emissiveTexture) {
            shaders.model.bindEmissiveTexture(primitive.material.emissiveTexture->getTexture());
        } else {
            shaders.model.bindEmissiveTexture(defaultEmissiveTexture);
        }

        if (primitive.material.normalTexture) {
            shaders.model.bindNormalTexture(primitive.material.normalTexture->getTexture());
        } else {
            shaders.model.bindNormalTexture(defaultNormalTexture);
        }

        if (primitive.material.ambientOcclusionTexture) {
            shaders.model.bindAmbientOcclusionTexture(primitive.material.ambientOcclusionTexture->getTexture());
        } else {
            shaders.model.bindAmbientOcclusionTexture(defaultAmbientOcclusionTexture);
        }

        if (primitive.material.metallicRoughnessTexture) {
            shaders.model.bindMetallicRoughnessTexture(primitive.material.metallicRoughnessTexture->getTexture());
        } else {
            shaders.model.bindMetallicRoughnessTexture(defaultMetallicRoughnessTexture);
        }

        shaders.model.draw(primitive.mesh);
    }
}
