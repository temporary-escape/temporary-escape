#include "Renderer.hpp"
#include "../Assets/AssetManager.hpp"
#include "../Utils/Random.hpp"

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

using namespace Engine;

struct Pos2UvVertex {
    Vector2 position;
    Vector2 uv;
};

static const std::array<Pos2UvVertex, 6> fullscreenQuad = {
    Pos2UvVertex{{-1.0f, -1.0f}, {0.0f, 1.0f}}, Pos2UvVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},
    Pos2UvVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},  Pos2UvVertex{{-1.0f, 1.0f}, {0.0f, 0.0f}},
    Pos2UvVertex{{1.0f, -1.0f}, {1.0f, 1.0f}},  Pos2UvVertex{{1.0f, 1.0f}, {1.0f, 0.0f}}};

Renderer::Renderer(const Config& config, Canvas2D& canvas, AssetManager& assetManager)
    : config(config), canvas(canvas), skyboxRenderer(config), shaders{ShaderSkybox{config},
                                                                      ShaderModel{config},
                                                                      ShaderGrid{config},
                                                                      ShaderBrdf{config},
                                                                      ShaderPbr{config},
                                                                      ShaderPlanetSurface{config},
                                                                      ShaderPlanetAtmosphere{config},
                                                                      ShaderParticleEmitter{config},
                                                                      ShaderBullet{config}} {

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
    createSSAOUbo();

    planet.surfaceTexture = assetManager.find<AssetTexture>("planet_surface_life");

    particleEmitter.vao = VertexArray();

    startTime = std::chrono::steady_clock::now();
}

Renderer::~Renderer() = default;

void Renderer::reloadShaders() {
    try {
        shaders.pbr = ShaderPbr(config);
        shaders.model = ShaderModel(config);
        shaders.planetSurface = ShaderPlanetSurface(config);
        shaders.planetAtmosphere = ShaderPlanetAtmosphere(config);
        shaders.particleEmitter = ShaderParticleEmitter(config);
        shaders.bullet = ShaderBullet(config);
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

void Renderer::createSSAOUbo() {
    SSAOUniform uniform{};
    std::mt19937_64 rng{};

    const auto lerp = [](float a, float b, float f) -> float { return a + f * (b - a); };

    for (auto i = 0; i < ssaoSamplesNum; i++) {
        Vector3 sample = {
            randomReal(rng, -1.0f, 1.0f),
            randomReal(rng, -1.0f, 1.0f),
            randomReal(rng, 0.0f, 1.0f),
        };

        sample *= randomReal(rng, 0.0f, 1.0f);

        auto scale = static_cast<float>(i) / static_cast<float>(ssaoSamplesNum);
        scale = lerp(0.1, 1.0, scale * scale);

        sample *= scale;

        uniform.samples[i] = Vector4{sample, 0.0f};
    }

    for (auto i = 0; i < ssaoNoiseNum; i++) {
        Vector3 noise = {
            randomReal(rng, -1.0f, 1.0f),
            randomReal(rng, -1.0f, 1.0f),
            0.0f,
        };

        uniform.noise[i] = Vector4{noise, 0.0f};
    }

    ssaoUbo = VertexBuffer(VertexBufferType::Uniform);
    ssaoUbo.bufferData(&uniform, sizeof(SSAOUniform), VertexBufferUsage::StaticDraw);
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
    const auto now = std::chrono::steady_clock::now();
    time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count()) / 1000.0f;

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
    updateBullets(scene);

    renderSceneBackground(viewport, scene);

    gBuffer.fbo.bind();
    renderScenePbr(viewport, scene);
    Framebuffer::DefaultFramebuffer.bind();

    // blit(viewport, gBuffer.fbo, Framebuffer::DefaultFramebuffer, FramebufferAttachment::Color3);

    renderPbr();
    renderSceneForward(viewport, scene);
    renderCanvas(viewport, scene);
}

void Renderer::blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target,
                    const FramebufferAttachment attachment, BufferBit bufferBit) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.getHandle());
    glReadBuffer(GLuint(attachment));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getHandle());
    glViewport(0, 0, viewport.x, viewport.y);
    glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GLenum(bufferBit), GL_NEAREST);
}

void Renderer::renderCanvas(const Vector2i& viewport, Scene& scene) {
    auto camera = getPrimaryCamera(scene);
    if (!camera) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    canvas.beginFrame(viewport);

    auto& componentSystemCanvasLines = scene.getComponentSystem<ComponentCanvasLines>();
    for (auto& component : componentSystemCanvasLines) {
        renderComponentCanvasLines(*camera, *component);
    }

    auto& componentSystemCanvasImage = scene.getComponentSystem<ComponentCanvasImage>();
    for (auto& component : componentSystemCanvasImage) {
        renderComponentCanvasImage(*camera, *component);
    }

    auto& componentSystemCanvasLabel = scene.getComponentSystem<ComponentCanvasLabel>();
    for (auto& component : componentSystemCanvasLabel) {
        renderComponentCanvasLabel(*camera, *component);
    }

    canvas.endFrame();
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
    shaders.pbr.bindSSAO(ssaoUbo);
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
    shaders.skybox.bindCameraUniform(cameraZeroPosUbo);

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
    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    blit(viewport, gBuffer.fbo, Framebuffer::DefaultFramebuffer, FramebufferAttachment::DepthStencil, BufferBit::Depth);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    auto& componentSystemParticleEmitter = scene.getComponentSystem<ComponentParticleEmitter>();

    shaders.particleEmitter.use();
    shaders.particleEmitter.bindCameraUniform(cameraUbo);
    shaders.particleEmitter.setTime(time);

    for (auto& component : componentSystemParticleEmitter) {
        renderComponentParticleEmitter(*component);
    }

    renderBullets();

    glDepthMask(GL_TRUE);
}

void Renderer::renderBullets() {
    if (!bullets.mesh) {
        return;
    }

    glEnable(GL_PROGRAM_POINT_SIZE);
    shaders.bullet.use();
    shaders.bullet.bindCameraUniform(cameraUbo);
    shaders.bullet.draw(bullets.mesh);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void Renderer::renderScenePbr(const Vector2i& viewport, Scene& scene) {
    auto& componentSystemModel = scene.getComponentSystem<ComponentModel>();
    auto& componentSystemTurret = scene.getComponentSystem<ComponentTurret>();
    auto& componentSystemGrid = scene.getComponentSystem<ComponentGrid>();
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

    for (auto& component : componentSystemTurret) {
        renderComponentTurret(*component);
    }

    shaders.grid.use();
    shaders.grid.bindCameraUniform(cameraUbo);

    for (auto& component : componentSystemGrid) {
        renderComponentGrid(*component);
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

void Renderer::updateBullets(Scene& scene) {
    const auto& data = scene.getBulletsData();

    if (bullets.size != data.size()) {
        if (!bullets.mesh) {
            bullets.mesh = Mesh{};
            bullets.vbo = VertexBuffer(VertexBufferType::Array);
            bullets.vbo.bufferData(data.data(), data.size() * sizeof(Scene::Bullet), VertexBufferUsage::DynamicDraw);

            bullets.mesh.addVertexBuffer(bullets.vbo, ShaderBullet::Position{}, ShaderBullet::Direction{},
                                         ShaderBullet::Color{}, ShaderBullet::Time{}, ShaderBullet::Speed{});
            bullets.mesh.setPrimitive(PrimitiveType::Points);
            bullets.mesh.setCount(data.size());
        } else {
            bullets.mesh.bind();
            bullets.vbo.bufferData(data.data(), data.size() * sizeof(Scene::Bullet), VertexBufferUsage::DynamicDraw);
        }
        bullets.size = data.size();
    } else if (bullets.mesh) {
        bullets.vbo.bufferSubData(data.data(), data.size() * sizeof(Scene::Bullet), 0);
    }
}

Camera* Renderer::getPrimaryCamera(Scene& scene) {
    auto& componentSystemCameraTurntable = scene.getComponentSystem<ComponentCameraTurntable>();
    auto& componentSystemCameraTop = scene.getComponentSystem<ComponentCameraTop>();

    Camera* primary = nullptr;

    for (auto& component : componentSystemCameraTurntable) {
        if (component->getPrimary()) {
            primary = component;
        }
    }

    for (auto& component : componentSystemCameraTop) {
        if (component->getPrimary()) {
            primary = component;
        }
    }

    return primary;
}

void Renderer::updateCameras(const Vector2i& viewport, Scene& scene) {
    auto& componentSystemCameraTurntable = scene.getComponentSystem<ComponentCameraTurntable>();
    auto& componentSystemCameraTop = scene.getComponentSystem<ComponentCameraTop>();

    for (auto& component : componentSystemCameraTurntable) {
        component->setViewport(viewport);
    }

    for (auto& component : componentSystemCameraTop) {
        component->setViewport(viewport);
    }

    auto primary = getPrimaryCamera(scene);

    const auto makeUniform = [&](const bool zero) {
        auto viewMatrix = primary->getViewMatrix();
        if (zero) {
            viewMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
        }
        const auto transformationProjectionMatrix = primary->getProjectionMatrix() * viewMatrix;
        const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);
        const auto projectionViewInverseMatrix = glm::inverse(transformationProjectionMatrix);

        CameraUniform cameraUniform{};
        cameraUniform.transformationProjectionMatrix = transformationProjectionMatrix;
        cameraUniform.viewProjectionInverseMatrix = projectionViewInverseMatrix;
        cameraUniform.viewMatrix = viewMatrix;
        cameraUniform.projectionMatrix = primary->getProjectionMatrix();
        cameraUniform.viewport = viewport;
        cameraUniform.eyesPos = eyesPos;

        return cameraUniform;
    };

    if (primary) {
        cameraViewMatrix = primary->getViewMatrix();
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

void Renderer::renderComponentParticleEmitter(ComponentParticleEmitter& component) {
    component.rebuild();

    const auto& transform = component.getObject().getTransform();
    shaders.particleEmitter.setModelMatrix(transform);
    shaders.particleEmitter.setModelViewMatrix(cameraViewMatrix * transform);
    shaders.particleEmitter.bindParticleTexture(component.getTexture()->getTexture());
    shaders.particleEmitter.bindParticleData(component.getUbo());
    particleEmitter.vao.bind();
    glDrawArrays(GL_POINTS, 0, component.getCount());
}

void Renderer::renderModel(const AssetModelPtr& model, const Matrix4& transform) {
    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));
    shaders.model.setModelMatrix(transform);
    shaders.model.setNormalMatrix(transformInverted);

    for (const auto& primitive : model->getPrimitives()) {
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

void Renderer::renderComponentModel(ComponentModel& component) {
    const auto& transform = component.getObject().getTransform();
    renderModel(component.getModel(), transform);
}

void Renderer::renderComponentTurret(ComponentTurret& component) {
    const auto& turret = component.getTurret()->getComponents();
    const auto transformInverted = glm::inverse(component.getObject().getTransform());

    auto transform = glm::translate(component.getObject().getTransform(), turret.base.offset);
    renderModel(turret.base.model, transform);

    const auto& rotation = component.getRotation();

    transform = glm::translate(component.getObject().getTransform(), turret.arm.offset);
    transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
    renderModel(turret.arm.model, transform);

    transform = glm::translate(component.getObject().getTransform(), turret.cannon.offset);
    transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, rotation.x, Vector3{1.0f, 0.0f, 0.0f});

    renderModel(turret.cannon.model, transform);
}

void Renderer::renderComponentGrid(ComponentGrid& component) {
    component.rebuild();

    const auto& transform = component.getObject().getTransform();
    shaders.grid.setModelMatrix(transform);

    const auto& meshes = component.getMeshes();

    for (const auto& [block, type] : meshes) {
        for (const auto& primitive : type.primitives) {
            shaders.grid.bindMaterialUniform(*primitive.ubo);

            if (primitive.material->baseColorTexture) {
                shaders.grid.bindBaseColorTexture(primitive.material->baseColorTexture->getTexture());
            } else {
                shaders.grid.bindBaseColorTexture(defaultBaseColorTexture);
            }

            if (primitive.material->emissiveTexture) {
                shaders.grid.bindEmissiveTexture(primitive.material->emissiveTexture->getTexture());
            } else {
                shaders.grid.bindEmissiveTexture(defaultEmissiveTexture);
            }

            if (primitive.material->normalTexture) {
                shaders.grid.bindNormalTexture(primitive.material->normalTexture->getTexture());
            } else {
                shaders.grid.bindNormalTexture(defaultNormalTexture);
            }

            if (primitive.material->ambientOcclusionTexture) {
                shaders.grid.bindAmbientOcclusionTexture(primitive.material->ambientOcclusionTexture->getTexture());
            } else {
                shaders.grid.bindAmbientOcclusionTexture(defaultAmbientOcclusionTexture);
            }

            if (primitive.material->metallicRoughnessTexture) {
                shaders.grid.bindMetallicRoughnessTexture(primitive.material->metallicRoughnessTexture->getTexture());
            } else {
                shaders.grid.bindMetallicRoughnessTexture(defaultMetallicRoughnessTexture);
            }

            shaders.grid.draw(primitive.mesh);
        }
    }
}

void Renderer::renderComponentCanvasImage(const Camera& camera, ComponentCanvasImage& component) {
    const auto& size = component.getSize();
    const auto& image = component.getImage();
    const auto& color = component.getColor();

    canvas.beginPath();
    const auto projected = camera.worldToScreen(component.getObject().getPosition()) + component.getOffset();
    canvas.rectImage(projected - size / 2.0f, size, image->getImage(), color);
    canvas.fill();
    canvas.closePath();
}

void Renderer::renderComponentCanvasLines(const Camera& camera, ComponentCanvasLines& component) {
    const auto width = component.getWidth();
    const auto& color = component.getColor();

    canvas.beginPath();
    canvas.strokeColor(color);
    canvas.strokeWidth(width);
    for (const auto& [from, to] : component.getLines()) {
        canvas.moveTo(camera.worldToScreen(from));
        canvas.lineTo(camera.worldToScreen(to));
    }
    canvas.stroke();
    canvas.closePath();
}

void Renderer::renderComponentCanvasLabel(const Camera& camera, ComponentCanvasLabel& component) {
    if (!component.getVisible()) {
        return;
    }

    canvas.beginPath();
    canvas.fillColor(component.getColor());
    canvas.fontFace(component.getFontFace()->getHandle());
    canvas.fontSize(component.getSize());
    auto projected = camera.worldToScreen(component.getObject().getPosition()) + component.getOffset();
    if (component.getCentered()) {
        projected -= canvas.textBounds(component.getText()) / 2.0f;
    }
    canvas.text(projected, component.getText());
    canvas.closePath();
}
