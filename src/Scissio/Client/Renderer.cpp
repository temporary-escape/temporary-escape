#include "Renderer.hpp"
#include "../Assets/AssetManager.hpp"

#define CMP "Renderer"
static const auto pi = 3.14159265358979323846f;
static const auto pi2 = 1.57079632679489661923f;

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

struct Pos2UvVertex {
    Vector2 position;
    Vector2 uv;
};

static const std::array<Pos2UvVertex, 6> fullscreenQuad = {
    Pos2UvVertex{{-1.0f, -1.0f}, {0.0f, 1.0f}}, Pos2UvVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},
    Pos2UvVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},  Pos2UvVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},
    Pos2UvVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},  Pos2UvVertex{{1.0f, 1.0f}, {1.0f, 0.0f}}};

Renderer::Renderer(const Config& config, Canvas2D& canvas, AssetManager& assetManager)
    : config(config), canvas(canvas),
      skyboxRenderer(config), shaders{ShaderSkybox{config},        ShaderModel{config},
                                      ShaderBrdf{config},          ShaderPbr{config},
                                      ShaderPlanetSurface{config}, ShaderPlanetAtmosphere{config}} {

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

    createFullScreenMesh();
    createBrdfTexture();
    createPlanetMesh();

    planet.surfaceTexture = assetManager.find<AssetTexture>("planet_surface_life");
}

Renderer::~Renderer() = default;

void Renderer::reloadShaders() {
    try {
        shaders.pbr = ShaderPbr(config);
        shaders.model = ShaderModel(config);
        shaders.planetSurface = ShaderPlanetSurface(config);
        shaders.planetAtmosphere = ShaderPlanetAtmosphere(config);
        Log::i(CMP, "Shaders reloaded");
    } catch (std::exception& e) {
        BACKTRACE(CMP, e, "Failed to reload shaders");
    }
}

void Renderer::createSkybox(uint64_t seed) {
    Log::i(CMP, "Creating skybox for seed {}", seed);

    if (!skybox.mesh) {
        skybox.mesh = Mesh{};

        VertexBuffer vbo(VertexBufferType::Array);
        vbo.bufferData(skyboxVertices, sizeof(skyboxVertices), VertexBufferUsage::StaticDraw);

        skybox.mesh.addVertexBuffer(std::move(vbo), ShaderSkybox::Position{});
        skybox.mesh.setCount(6 * 2 * 3);
        skybox.mesh.setPrimitive(PrimitiveType::Triangles);
    }

    skybox.seed = seed;

    skybox.textures = skyboxRenderer.renderAndFilter(seed);
}

void Renderer::createFullScreenMesh() {
    fullScreenMesh = Mesh{};

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(fullscreenQuad.data(), fullscreenQuad.size() * sizeof(fullscreenQuad),
                   VertexBufferUsage::StaticDraw);

    fullScreenMesh.addVertexBuffer(std::move(vbo), ShaderPbr::Position{}, ShaderPbr::TextureCoordinates{});
    fullScreenMesh.setCount(6);
    fullScreenMesh.setPrimitive(PrimitiveType::Triangles);
}

void Renderer::createBrdfTexture() {
    Mesh mesh{};

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(fullscreenQuad.data(), fullscreenQuad.size() * sizeof(fullscreenQuad),
                   VertexBufferUsage::StaticDraw);

    mesh.addVertexBuffer(std::move(vbo), ShaderBrdf::Position{}, ShaderBrdf::TextureCoordinates{});
    mesh.setCount(6);
    mesh.setPrimitive(PrimitiveType::Triangles);

    const auto size = Vector2i{config.brdfSize, config.brdfSize};

    Framebuffer fbo;
    Renderbuffer rbo;

    fbo.bind();
    glViewport(0, 0, size.x, size.y);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    glDisable(GL_BLEND);

    brdf.texture = Texture2D{};
    brdf.texture.setStorage(0, size, PixelType::Rg16f);
    fbo.attach(brdf.texture, FramebufferAttachment::Color0, 0);

    rbo.setStorage(size, PixelType::Depth24Stencil8);
    fbo.attach(rbo, FramebufferAttachment::DepthStencil);

    shaders.brdf.use();
    shaders.brdf.draw(mesh);

    Framebuffer::DefaultFramebuffer.bind();
}

void Renderer::createPlanetMesh() {
#pragma pack(push, 1)
    struct Vertex {
        Vector3 pos;
        Vector3 normal;
    };
#pragma pack(pop)

    static_assert(sizeof(Vertex) == 6 * sizeof(float), "Size of Vertex struct must be 6 floats");

    // Source: https://stackoverflow.com/a/5989676
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    static const auto radius = 0.5f;
    static const auto rings = 32;
    static const auto sectors = 64;

    const auto R = 1.0f / static_cast<float>(rings - 1);
    const auto S = 1.0f / static_cast<float>(sectors - 1);

    vertices.resize(rings * sectors);
    indices.resize(rings * sectors * 6);

    auto v = vertices.begin();
    for (auto r = 0; r < rings; r++)
        for (auto s = 0; s < sectors; s++) {
            float const y = std::sin(-pi2 + pi * r * R);
            float const x = std::cos(2 * pi * s * S) * std::sin(pi * r * R);
            float const z = std::sin(2 * pi * s * S) * std::sin(pi * r * R);

            *v++ = Vertex{Vector3{x, y, z} * radius, Vector3{x, y, z}};
        }

    auto i = indices.begin();
    for (auto r = 0; r < rings; r++) {
        for (auto s = 0; s < sectors; s++) {
            *i++ = r * sectors + s;
            *i++ = r * sectors + (s + 1);
            *i++ = (r + 1) * sectors + (s + 1);

            *i++ = r * sectors + s;
            *i++ = (r + 1) * sectors + (s + 1);
            *i++ = (r + 1) * sectors + s;
        }
    }

    planet.mesh = Mesh{};

    auto vbo = VertexBuffer(VertexBufferType::Array);
    vbo.bufferData(vertices.data(), vertices.size() * sizeof(Vertex), VertexBufferUsage::StaticDraw);

    planet.mesh.addVertexBuffer(std::move(vbo), ShaderPlanetSurface::Position{}, ShaderPlanetSurface::Normal{});

    auto ibo = VertexBuffer(VertexBufferType::Indices);
    ibo.bufferData(indices.data(), indices.size() * sizeof(uint16_t), VertexBufferUsage::StaticDraw);

    planet.mesh.setIndexBuffer(std::move(ibo), IndexType::UnsignedShort);

    planet.mesh.setCount(static_cast<int>(indices.size()));
    planet.mesh.setPrimitive(PrimitiveType::Triangles);
}

void Renderer::render(const Vector2i& viewport, Scene& scene) {
    // const auto t0 = std::chrono::steady_clock::now();

    if (gBuffer.size != viewport) {
        gBuffer.fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
        gBuffer.fboColorAlpha.setStorage(0, viewport, PixelType::Rgba8u);
        gBuffer.fboEmissive.setStorage(0, viewport, PixelType::Rgb8u);
        gBuffer.fboMetallicRoughnessAmbient.setStorage(0, viewport, PixelType::Rgb8u);
        gBuffer.fboNormal.setStorage(0, viewport, PixelType::Rgb16f);

        if (!gBuffer.fboInit) {
            gBuffer.fboInit = true;
            gBuffer.fbo.attach(gBuffer.fboColorAlpha, FramebufferAttachment::Color0, 0);
            gBuffer.fbo.attach(gBuffer.fboEmissive, FramebufferAttachment::Color1, 0);
            gBuffer.fbo.attach(gBuffer.fboMetallicRoughnessAmbient, FramebufferAttachment::Color2, 0);
            gBuffer.fbo.attach(gBuffer.fboNormal, FramebufferAttachment::Color3, 0);
            gBuffer.fbo.attach(gBuffer.fboDepth, FramebufferAttachment::DepthStencil, 0);
        }
    }

    updateLights(scene);
    updateCameras(viewport, scene);

    renderSceneBackground(viewport, scene);

    gBuffer.fbo.bind();
    renderScenePbr(viewport, scene);
    Framebuffer::DefaultFramebuffer.bind();

    // blit(viewport, gBuffer.fbo, Framebuffer::DefaultFramebuffer, FramebufferAttachment::Color3);

    // renderPbr();

    /*glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);*/

    /*canvas.beginFrame(viewport);
    gui.reset();

    widgetDebugStats.render();

    gui.render(viewport);
    canvas.endFrame();*/

    /*const auto t1 = std::chrono::steady_clock::now();
    const auto tDiff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    client.getStats().render.frameTimeMs.store(tDiff.count());*/
}

void Renderer::blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target,
                    const FramebufferAttachment attachment) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.getHandle());
    glReadBuffer(GLuint(attachment));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getHandle());
    glViewport(0, 0, viewport.x, viewport.y);
    glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Renderer::renderPbr() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.pbr.use();
    shaders.pbr.bindDepthTexture(gBuffer.fboDepth);
    shaders.pbr.bindBaseColorTexture(gBuffer.fboColorAlpha);
    shaders.pbr.bindEmissiveTexture(gBuffer.fboEmissive);
    shaders.pbr.bindMetallicRoughnessAmbientTexture(gBuffer.fboMetallicRoughnessAmbient);
    shaders.pbr.bindNormalTexture(gBuffer.fboNormal);
    shaders.pbr.bindBrdfTexture(brdf.texture);
    shaders.pbr.bindSkyboxIrradianceTexture(skybox.textures.irradiance);
    shaders.pbr.bindSkyboxPrefilterTexture(skybox.textures.prefilter);
    shaders.pbr.bindCameraUniform(cameraUbo);
    shaders.pbr.bindDirectionalLightsUniform(directionalLightsUbo);
    shaders.pbr.draw(fullScreenMesh);
}

void Renderer::renderSceneBackground(const Vector2i& viewport, Scene& scene) {
    glViewport(0, 0, viewport.x, viewport.y);
    Framebuffer::DefaultFramebuffer.bind();
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    auto& componentSystemSkybox = scene.getComponentSystem<ComponentSkybox>();
    auto& componentSystemPlanet = scene.getComponentSystem<ComponentPlanet>();

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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    shaders.skybox.use();
    shaders.skybox.bindCameraUniform(cameraUbo);

    if (componentSkybox != componentSystemSkybox.end()) {
        renderComponentSkybox(**componentSkybox);
    }

    shaders.planetSurface.use();
    shaders.planetSurface.bindCameraUniform(cameraUbo);
    shaders.planetSurface.bindDirectionalLightsUniform(directionalLightsUbo);

    for (auto& component : componentSystemPlanet) {
        renderComponentPlanetSurface(*component);
    }

    // glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    shaders.planetAtmosphere.use();
    shaders.planetAtmosphere.bindCameraUniform(cameraUbo);
    shaders.planetAtmosphere.bindDirectionalLightsUniform(directionalLightsUbo);

    for (auto& component : componentSystemPlanet) {
        renderComponentPlanetAtmosphere(*component);
    }
}

void Renderer::renderSceneForward(const Vector2i& viewport, Scene& scene) {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::renderScenePbr(const Vector2i& viewport, Scene& scene) {
    auto& componentSystemModel = scene.getComponentSystem<ComponentModel>();
    auto& componentSystemPlanet = scene.getComponentSystem<ComponentPlanet>();

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
    // glClearBufferfv(GL_COLOR, 3, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    shaders.model.use();
    shaders.model.bindCameraUniform(cameraUbo);

    for (auto& component : componentSystemModel) {
        renderComponentModel(*component);
    }
}

void Renderer::updateLights(Scene& scene) {
    auto& componentSystemDirectionalLights = scene.getComponentSystem<ComponentDirectionalLight>();

    DirectionalLightsUniform directionalLightsUniform{};

    for (const auto& component : componentSystemDirectionalLights) {
        if (directionalLightsUniform.count >= maxDirectionalLights) {
            break;
        }

        const auto dir = glm::normalize(component->getObject().getPosition());
        const auto& color = component->getColor();

        directionalLightsUniform.directions[directionalLightsUniform.count] = Vector4{dir, 0.0f};
        directionalLightsUniform.colors[directionalLightsUniform.count] = color;
        directionalLightsUniform.count++;
    }

    if (!directionalLightsUbo) {
        directionalLightsUbo = VertexBuffer(VertexBufferType::Uniform);
        directionalLightsUbo.bufferData(&directionalLightsUniform, sizeof(directionalLightsUniform),
                                        VertexBufferUsage::DynamicDraw);
    } else {
        directionalLightsUbo.bufferSubData(&directionalLightsUniform, sizeof(directionalLightsUniform), 0);
    }
}

void Renderer::updateCameras(const Vector2i& viewport, Scene& scene) {
    auto& componentSystemCameras = scene.getComponentSystem<ComponentCamera>();

    ComponentCamera* primary = nullptr;
    for (auto& component : componentSystemCameras) {
        component->setProjection(viewport, 70.0f);
        if (component->getPrimary()) {
            primary = component;
        }
    }

    const auto makeUniform = [&](const bool zero) {
        auto viewMatrix = primary->getViewMatrix();
        if (zero) {
            viewMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
        }
        const auto transformationProjectionMatrix = primary->getProjectionMatrix() * viewMatrix;
        const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);
        const auto projectionViewInverseMatrix = glm::inverse(transformationProjectionMatrix);

        CameraUniform cameraUniform;
        cameraUniform.transformationProjectionMatrix = transformationProjectionMatrix;
        cameraUniform.viewProjectionInverseMatrix = projectionViewInverseMatrix;
        cameraUniform.viewport = viewport;
        cameraUniform.eyesPos = eyesPos;

        return cameraUniform;
    };

    if (primary) {
        CameraUniform cameraUniform = makeUniform(false);

        if (!cameraUbo) {
            cameraUbo = VertexBuffer(VertexBufferType::Uniform);
            cameraUbo.bufferData(&cameraUniform, sizeof(CameraUniform), VertexBufferUsage::DynamicDraw);
        } else {
            cameraUbo.bufferSubData(&cameraUniform, sizeof(CameraUniform), 0);
        }
    }

    if (primary) {
        CameraUniform cameraUniform = makeUniform(true);

        if (!cameraZeroPosUbo) {
            cameraZeroPosUbo = VertexBuffer(VertexBufferType::Uniform);
            cameraZeroPosUbo.bufferData(&cameraUniform, sizeof(CameraUniform), VertexBufferUsage::DynamicDraw);
        } else {
            cameraZeroPosUbo.bufferSubData(&cameraUniform, sizeof(CameraUniform), 0);
        }
    }
}

void Renderer::renderComponentSkybox(ComponentSkybox& component) {
    shaders.skybox.setModelMatrix(component.getObject().getTransform());
    shaders.skybox.bindSkyboxTexture(skybox.textures.texture);
    shaders.skybox.draw(skybox.mesh);
}

void Renderer::renderComponentPlanetSurface(ComponentPlanet& component) {
    const auto& transform = component.getObject().getTransform();
    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));
    shaders.planetSurface.setModelMatrix(transform);
    shaders.planetSurface.setNormalMatrix(transformInverted);
    shaders.planetSurface.bindSurfaceTexture(planet.surfaceTexture->getTexture());
    shaders.planetSurface.draw(planet.mesh);
}

void Renderer::renderComponentPlanetAtmosphere(ComponentPlanet& component) {
    const auto& transform = component.getObject().getTransform();
    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));
    shaders.planetAtmosphere.setModelMatrix(transform);
    shaders.planetAtmosphere.setNormalMatrix(transformInverted);
    shaders.planetAtmosphere.draw(planet.mesh);
}

void Renderer::renderComponentModel(ComponentModel& component) {
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
