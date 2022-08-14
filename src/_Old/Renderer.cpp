#include "Renderer.hpp"
#include "../Assets/AssetManager.hpp"
#include "../Utils/Random.hpp"

#define CMP "Renderer"

using namespace Engine;

union ObjectId {
    uint8_t data[2];
    uint16_t id{0};
};

static Color4 entityIdToObjectIdColor(const EntityPtr& entity) {
    ObjectId id;
    id.id = entity->getId();
    Color4 color;
    color.r = static_cast<float>(id.data[0]) / 255.0f;
    color.g = static_cast<float>(id.data[1]) / 255.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    return color;
}

static float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

Renderer::Renderer(const Engine::Config& config, const Engine::Shaders& shaders, SkyboxRenderer& skyboxRenderer,
                   Grid::Builder& gridBuilder)
    : config(config), shaders(shaders), skyboxRenderer(skyboxRenderer), gridBuilder(gridBuilder) {

    const auto setColor = [](Texture2D& texture, const Color4& color) {
        std::unique_ptr<uint8_t[]> pixels(new uint8_t[8 * 8 * 4]);
        for (size_t i = 0; i < 8 * 8 * 4; i += 4) {
            pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
            pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
            pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
            pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
        }

        texture = Texture2D{};
        texture.setStorage(0, {8, 8}, PixelType::Rgba8u);
        texture.setPixels(0, {0, 0}, {8, 8}, PixelType::Rgba8u, pixels.get());
    };

    const auto setColorCubemap = [](TextureCubemap& texture, const Color4& color) {
        std::unique_ptr<uint8_t[]> pixels(new uint8_t[8 * 8 * 4]);
        for (size_t i = 0; i < 8 * 8 * 4; i += 4) {
            pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
            pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
            pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
            pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
        }

        texture = TextureCubemap{};
        texture.setStorage(0, {8, 8}, PixelType::Rgba8u);
        for (const auto side : TextureCubemap::sides) {
            texture.setPixels(0, {0, 0}, side, {8, 8}, PixelType::Rgba8u, pixels.get());
        }
    };

    setColor(defaultTextures.baseColorTexture, Color4{1.0f, 0.0f, 1.0f, 1.0f});
    setColor(defaultTextures.normalTexture, Color4{0.5f, 0.5f, 1.0f, 1.0f});
    setColor(defaultTextures.emissiveTexture, Color4{0.0f, 0.0f, 0.0f, 1.0f});
    setColor(defaultTextures.metallicRoughnessTexture, Color4{0.0f, 0.5f, 0.5f, 1.0f});
    setColor(defaultTextures.ambientOcclusionTexture, Color4{1.0f, 1.0f, 1.0f, 1.0f});
    setColorCubemap(defaultTextures.skyboxTexture, Color4{0.5f, 0.5f, 0.5f, 1.0f});

    createBrdfTexture();

    particleEmitter.vao = VertexArray();

    ssao.noise = generateSSAONoise();
    ssao.samples = generateSSAOSamples();

    startTime = std::chrono::steady_clock::now();
}

void Renderer::render(Scene& scene) {
    const auto now = std::chrono::steady_clock::now();
    time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count()) / 1000.0f;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    if (!state.gBuffer) {
        EXCEPTION(CMP, "No GBuffer set");
    }

    if (state.viewport.x == 0 || state.viewport.y == 0) {
        EXCEPTION(CMP, "Viewport is invalid");
    }

    const auto camera = scene.getPrimaryCamera();
    if (!camera) {
        EXCEPTION(CMP, "No camera in scene");
    }

    camera->setViewport(state.viewport);
    setCamera(*camera);

    {
        auto& directionalLights = scene.getComponentSystem<ComponentDirectionalLight>();
        std::vector<DirectionalLight> lights;
        for (const auto& component : directionalLights) {
            lights.push_back({component->getColor(), glm::normalize(component->getObject().getPosition())});
        }
        setDirectionalLights(lights);
    }

    auto entitySkybox = scene.getSkybox();
    if (entitySkybox) {
        auto component = entitySkybox->getComponent<ComponentSkybox>();
        component->recalculate(skyboxRenderer);
        state.skybox = &component->getSkybox();
        state.skyboxTransform = entitySkybox->getTransform();
    } else {
        state.skybox = nullptr;
        state.skyboxTransform = glm::scale(Matrix4{1.0f}, Vector3{100.0f});
    }

    state.gBuffer->fbo.bind();
    renderSceneDeffered(scene);

    glDisable(GL_CULL_FACE);

    state.gBuffer->fboAo.bind();
    renderSSAO();

    state.gBuffer->fboFront.bind();
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);

    if (state.renderBackground) {
        renderSceneBackground(scene);
    }
    renderPbrBuffer();
    renderSceneForward(scene);
    // renderDebugNormals(scene);

    state.gBuffer->fboFxaa.bind();
    renderFXAA();

    if (state.renderBloom) {
        state.gBuffer->fboBloomExtract.bind();
        renderBloomExtract();

        state.gBuffer->fboBloomBlurVertical.bind();
        renderBloomBlurVertical();

        state.gBuffer->fboBloomBlurHorizontal.bind();
        renderBloomBlurHorizontal();

        state.gBuffer->fboBloomCombine.bind();
        renderBloomCombine();
    }
}

void Renderer::renderSceneDeffered(Scene& scene) {
    auto& componentSystemModel = scene.getComponentSystem<ComponentModel>();
    auto& componentSystemGrid = scene.getComponentSystem<ComponentGrid>();
    auto& componentSystemTurret = scene.getComponentSystem<ComponentTurret>();

    GLuint attachments[5] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4,
    };
    glDrawBuffers(5, attachments);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);

    glViewport(0, 0, state.viewport.x, state.viewport.y);
    static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfv(GL_COLOR, 3, &black.x);
    glClearBufferfv(GL_COLOR, 4, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    shaders.model.use();
    shaders.model.bindCameraUniform(state.cameraUbo);

    for (auto& component : componentSystemModel) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentModel(*component);
    }

    for (auto& component : componentSystemTurret) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentTurret(*component);
    }

    shaders.grid.use();
    shaders.grid.bindCameraUniform(state.cameraUbo);

    for (auto& component : componentSystemGrid) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentGrid(*component);
    }
}

void Renderer::renderPbrBuffer() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.pbr.use();
    shaders.pbr.bindDepthTexture(state.gBuffer->fboDepth);
    shaders.pbr.bindBaseColorTexture(state.gBuffer->fboColorAlpha);
    shaders.pbr.bindEmissiveTexture(state.gBuffer->fboEmissive);
    shaders.pbr.bindMetallicRoughnessAmbientTexture(state.gBuffer->fboMetallicRoughnessAmbient);
    shaders.pbr.bindNormalTexture(state.gBuffer->fboNormal);
    shaders.pbr.bindBrdfTexture(brdf.texture);
    if (state.skybox) {
        shaders.pbr.bindSkyboxIrradianceTexture(state.skybox->irradiance);
        shaders.pbr.bindSkyboxPrefilterTexture(state.skybox->prefilter);
    } else {
        shaders.pbr.bindSkyboxIrradianceTexture(defaultTextures.skyboxTexture);
        shaders.pbr.bindSkyboxIrradianceTexture(defaultTextures.skyboxTexture);
    }
    shaders.pbr.bindSSAOTexture(state.gBuffer->fboAoResult);
    shaders.pbr.bindCameraUniform(state.cameraUbo);
    shaders.pbr.bindDirectionalLightsUniform(state.directionalLightsUbo);
    shaders.pbr.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderSSAO() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.ssao.use();
    shaders.ssao.bindDepthTexture(state.gBuffer->fboDepth);
    shaders.ssao.bindNormalTexture(state.gBuffer->fboNormal);
    shaders.ssao.bindCameraUniform(state.cameraUbo);
    shaders.ssao.setSamples(ssao.samples);
    shaders.ssao.bindNoiseTexture(ssao.noise);
    shaders.ssao.setNoiseScale(Vector2{state.viewport} / Vector2{4.0f});
    shaders.ssao.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderFXAA() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.fxaa.use();
    shaders.fxaa.setInputSize(state.viewport);
    shaders.fxaa.setOutputSize(state.viewport);
    shaders.fxaa.setTextureSize(state.viewport);
    shaders.fxaa.setFrameCount(0);
    shaders.fxaa.setFrameDirection(0);
    shaders.fxaa.setMvpMatrix(Matrix4{1.0f});
    shaders.fxaa.bindTexture(state.gBuffer->fboFrontColor);
    shaders.fxaa.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderBloomExtract() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.bloomExtract.use();
    shaders.bloomExtract.setBrightness(1.0);
    shaders.bloomExtract.bindTexture(state.gBuffer->fboFrontColor);
    shaders.bloomExtract.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderBloomBlurVertical() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.bloomBlur.use();
    shaders.bloomBlur.setHorizontal(false);
    shaders.bloomBlur.setSize(8);
    shaders.bloomBlur.bindTexture(state.gBuffer->fboBloomColor);
    shaders.bloomBlur.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderBloomBlurHorizontal() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.bloomBlur.use();
    shaders.bloomBlur.setHorizontal(true);
    shaders.bloomBlur.setSize(8);
    shaders.bloomBlur.bindTexture(state.gBuffer->fboBloomBlurResult);
    shaders.bloomBlur.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderBloomCombine() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.bloomCombine.use();
    shaders.bloomCombine.bindColorTexture(state.gBuffer->fboFrontColor);
    shaders.bloomCombine.bindBloomTexture(state.gBuffer->fboBloomColor);
    shaders.bloomCombine.draw(meshes.fullScreenQuad);

    glDepthMask(GL_TRUE);
}

void Renderer::renderSceneBackground(Scene& scene) {
    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    shaders.skybox.use();
    shaders.skybox.bindCameraUniform(state.cameraZeroPosUbo);

    renderSkybox(state.skybox ? state.skybox->texture : defaultTextures.skyboxTexture, state.skyboxTransform);

    glDepthMask(GL_TRUE);
}

void Renderer::renderSceneForward(Scene& scene) {
    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    auto& componentSystemParticleEmitter = scene.getComponentSystem<ComponentParticleEmitter>();

    shaders.particleEmitter.use();
    shaders.particleEmitter.bindCameraUniform(state.cameraUbo);
    shaders.particleEmitter.setTime(time);

    for (auto& component : componentSystemParticleEmitter) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentParticleEmitter(*component);
    }

    auto& componentSystemPolyShape = scene.getComponentSystem<ComponentPolyShape>();

    shaders.polyShape.use();
    shaders.polyShape.bindCameraUniform(state.cameraUbo);

    for (auto& component : componentSystemPolyShape) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentPolyShape(*component);
    }

    auto& componentSystemLines = scene.getComponentSystem<ComponentLines>();

    shaders.lines.use();
    shaders.lines.bindCameraUniform(state.cameraUbo);

    for (auto& component : componentSystemLines) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentLines(*component);
    }

    auto& componentSystemPointCloud = scene.getComponentSystem<ComponentPointCloud>();

    shaders.pointCloud.use();
    shaders.pointCloud.bindCameraUniform(state.cameraUbo);

    for (auto& component : componentSystemPointCloud) {
        if (!component->getObject().isVisible()) {
            continue;
        }
        renderComponentPointCloud(*component);
    }

    glDepthMask(GL_TRUE);
}

void Renderer::renderSkybox(const TextureCubemap& cubemap, const Matrix4& transform) {
    shaders.skybox.setModelMatrix(transform);
    shaders.skybox.bindSkyboxTexture(cubemap);
    shaders.skybox.draw(meshes.skybox);
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
            shaders.model.bindBaseColorTexture(defaultTextures.baseColorTexture);
        }

        if (primitive.material.emissiveTexture) {
            shaders.model.bindEmissiveTexture(primitive.material.emissiveTexture->getTexture());
        } else {
            shaders.model.bindEmissiveTexture(defaultTextures.emissiveTexture);
        }

        if (primitive.material.normalTexture) {
            shaders.model.bindNormalTexture(primitive.material.normalTexture->getTexture());
        } else {
            shaders.model.bindNormalTexture(defaultTextures.normalTexture);
        }

        if (primitive.material.ambientOcclusionTexture) {
            shaders.model.bindAmbientOcclusionTexture(primitive.material.ambientOcclusionTexture->getTexture());
        } else {
            shaders.model.bindAmbientOcclusionTexture(defaultTextures.ambientOcclusionTexture);
        }

        if (primitive.material.metallicRoughnessTexture) {
            shaders.model.bindMetallicRoughnessTexture(primitive.material.metallicRoughnessTexture->getTexture());
        } else {
            shaders.model.bindMetallicRoughnessTexture(defaultTextures.metallicRoughnessTexture);
        }

        shaders.model.draw(primitive.mesh);
    }
}

void Renderer::renderComponentModel(ComponentModel& component) {
    const auto transform = component.getObject().getAbsoluteTransform();
    if (auto entity = component.getObject().asEntity()) {
        shaders.model.setObjectId(entityIdToObjectIdColor(entity));
    }
    renderModel(component.getModel(), transform);
}

void Renderer::renderComponentGrid(ComponentGrid& component) {
    component.recalculate(gridBuilder);
    const auto transform = component.getObject().getAbsoluteTransform();

    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));
    shaders.grid.setModelMatrix(transform);
    shaders.grid.setNormalMatrix(transformInverted);

    if (auto entity = component.getObject().asEntity()) {
        shaders.grid.setObjectId(entityIdToObjectIdColor(entity));
    }

    for (const auto& primitive : component.getPrimitives()) {
        shaders.grid.bindMaterialUniform(primitive.ubo);

        if (primitive.material.baseColorTexture) {
            shaders.model.bindBaseColorTexture(primitive.material.baseColorTexture->getTexture());
        } else {
            shaders.model.bindBaseColorTexture(defaultTextures.baseColorTexture);
        }

        if (primitive.material.emissiveTexture) {
            shaders.model.bindEmissiveTexture(primitive.material.emissiveTexture->getTexture());
        } else {
            shaders.model.bindEmissiveTexture(defaultTextures.emissiveTexture);
        }

        if (primitive.material.normalTexture) {
            shaders.model.bindNormalTexture(primitive.material.normalTexture->getTexture());
        } else {
            shaders.model.bindNormalTexture(defaultTextures.normalTexture);
        }

        if (primitive.material.ambientOcclusionTexture) {
            shaders.model.bindAmbientOcclusionTexture(primitive.material.ambientOcclusionTexture->getTexture());
        } else {
            shaders.model.bindAmbientOcclusionTexture(defaultTextures.ambientOcclusionTexture);
        }

        if (primitive.material.metallicRoughnessTexture) {
            shaders.model.bindMetallicRoughnessTexture(primitive.material.metallicRoughnessTexture->getTexture());
        } else {
            shaders.model.bindMetallicRoughnessTexture(defaultTextures.metallicRoughnessTexture);
        }

        shaders.grid.draw(primitive.mesh);
    }
}

void Renderer::renderComponentTurret(ComponentTurret& component) {
    const auto& turret = component.getTurret()->getComponents();
    const auto absoluteTransform = component.getObject().getAbsoluteTransform();
    const auto transformInverted = glm::inverse(absoluteTransform);

    auto transform = glm::translate(absoluteTransform, turret.base.offset);
    renderModel(turret.base.model, transform);

    const auto& rotation = component.getRotation();

    transform = glm::translate(absoluteTransform, turret.arm.offset);
    transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
    renderModel(turret.arm.model, transform);

    transform = glm::translate(absoluteTransform, turret.cannon.offset);
    transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, rotation.x, Vector3{1.0f, 0.0f, 0.0f});

    renderModel(turret.cannon.model, transform);
}

void Renderer::renderComponentParticleEmitter(ComponentParticleEmitter& component) {
    component.rebuild();

    const auto transform = component.getObject().getAbsoluteTransform();
    shaders.particleEmitter.setModelMatrix(transform);
    shaders.particleEmitter.setModelViewMatrix(state.cameraViewMatrix * transform);
    shaders.particleEmitter.bindParticleTexture(component.getTexture()->getTexture());
    shaders.particleEmitter.bindParticleData(component.getUbo());
    particleEmitter.vao.bind();
    glDrawArrays(GL_POINTS, 0, component.getCount());
}

void Renderer::renderComponentLines(ComponentLines& component) {
    component.recalculate();

    const auto transform = component.getObject().getAbsoluteTransform();
    shaders.lines.setModelMatrix(transform);
    shaders.lines.draw(component.getMesh());
}

void Renderer::renderComponentPolyShape(ComponentPolyShape& component) {
    component.recalculate();

    const auto transform = component.getObject().getAbsoluteTransform();
    shaders.polyShape.setModelMatrix(transform);
    shaders.polyShape.draw(component.getMesh());
}

void Renderer::renderComponentPointCloud(ComponentPointCloud& component) {
    component.recalculate();

    const auto transform = component.getObject().getAbsoluteTransform();
    shaders.pointCloud.setModelMatrix(transform);
    shaders.pointCloud.bindTexture(component.getTexture()->getTexture());
    shaders.pointCloud.draw(component.getMesh());
}

void Renderer::setCamera(Camera& camera) {
    const auto makeUniform = [&](const bool zero) {
        auto viewMatrix = camera.getViewMatrix();
        if (zero) {
            viewMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
        }
        const auto transformationProjectionMatrix = camera.getProjectionMatrix() * viewMatrix;
        const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);
        const auto projectionViewInverseMatrix = glm::inverse(transformationProjectionMatrix);

        CameraUniform cameraUniform{};
        cameraUniform.transformationProjectionMatrix = transformationProjectionMatrix;
        cameraUniform.viewProjectionInverseMatrix = projectionViewInverseMatrix;
        cameraUniform.viewMatrix = viewMatrix;
        cameraUniform.projectionMatrix = camera.getProjectionMatrix();
        cameraUniform.viewport = state.viewport;
        cameraUniform.eyesPos = eyesPos;

        return cameraUniform;
    };

    state.cameraViewMatrix = camera.getViewMatrix();

    {
        CameraUniform cameraUniform = makeUniform(false);

        if (!state.cameraUbo) {
            state.cameraUbo = VertexBuffer(VertexBufferType::Uniform);
            state.cameraUbo.bufferData(&cameraUniform, sizeof(CameraUniform), VertexBufferUsage::DynamicDraw);
        } else {
            state.cameraUbo.bufferSubData(&cameraUniform, sizeof(CameraUniform), 0);
        }
    }

    {
        CameraUniform cameraUniform = makeUniform(true);

        if (!state.cameraZeroPosUbo) {
            state.cameraZeroPosUbo = VertexBuffer(VertexBufferType::Uniform);
            state.cameraZeroPosUbo.bufferData(&cameraUniform, sizeof(CameraUniform), VertexBufferUsage::DynamicDraw);
        } else {
            state.cameraZeroPosUbo.bufferSubData(&cameraUniform, sizeof(CameraUniform), 0);
        }
    }
}

void Renderer::setDirectionalLights(const std::vector<DirectionalLight>& lights) {
    DirectionalLightsUniform directionalLightsUniform{};

    for (const auto& light : lights) {
        if (directionalLightsUniform.count >= maxDirectionalLights) {
            break;
        }

        const auto dir = glm::normalize(light.direction);

        directionalLightsUniform.directions[directionalLightsUniform.count] = Vector4{dir, 0.0f};
        directionalLightsUniform.colors[directionalLightsUniform.count] = light.color;
        directionalLightsUniform.count++;
    }

    if (!state.directionalLightsUbo) {
        state.directionalLightsUbo = VertexBuffer(VertexBufferType::Uniform);
        state.directionalLightsUbo.bufferData(&directionalLightsUniform, sizeof(directionalLightsUniform),
                                              VertexBufferUsage::DynamicDraw);
    } else {
        state.directionalLightsUbo.bufferSubData(&directionalLightsUniform, sizeof(directionalLightsUniform), 0);
    }
}

void Renderer::createBrdfTexture() {
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
    shaders.brdf.draw(meshes.fullScreenQuad);

    Framebuffer::DefaultFramebuffer.bind();
}

std::vector<Vector3> Renderer::generateSSAOSamples() {
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    return ssaoKernel;
}

Texture2D Renderer::generateSSAONoise() {
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0,
                        0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }

    Texture2D texture;
    texture.setStorage(0, Vector2i{4, 4}, PixelType::Rgb32f);
    texture.setPixels(0, Vector2i{0, 0}, Vector2i{4, 4}, PixelType::Rgb32f, ssaoNoise.data());
    texture.setWrapping(TextureWrapping::Repeat, TextureWrapping::Repeat);
    texture.setFiltering(TextureFiltering::Nearest, TextureFiltering::Nearest);
    texture.setMipMapLevel(0, 0);
    return texture;
}

/*static const auto pi = 3.14159265358979323846f;
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

union ObjectId {
    uint8_t data[2];
    uint16_t id{0};
};

static Color4 entityIdToObjectIdColor(const EntityPtr& entity) {
    ObjectId id;
    id.id = entity->getId();
    Color4 color;
    color.r = static_cast<float>(id.data[0]) / 255.0f;
    color.g = static_cast<float>(id.data[1]) / 255.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    return color;
}

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
                                                                      ShaderBullet{config},
                                                                      ShaderFXAA{config},
                                                                      ShaderPointCloud{config},
                                                                      ShaderLines{config}} {

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

    const auto setColorCubemap = [](TextureCubemap& texture, const Color4& color) {
        std::unique_ptr<uint8_t[]> pixels(new uint8_t[8 * 8 * 4]);
        for (size_t i = 0; i < 8 * 8 * 4; i += 4) {
            pixels[i + 0] = static_cast<uint8_t>(color.r * 255.0f);
            pixels[i + 1] = static_cast<uint8_t>(color.g * 255.0f);
            pixels[i + 2] = static_cast<uint8_t>(color.b * 255.0f);
            pixels[i + 3] = static_cast<uint8_t>(color.a * 255.0f);
        }

        texture.setStorage(0, {8, 8}, PixelType::Rgba8u);
        for (const auto side : TextureCubemap::sides) {
            texture.setPixels(0, {0, 0}, side, {8, 8}, PixelType::Rgba8u, pixels.get());
        }
    };

    setColor(defaultBaseColorTexture, Color4{1.0f, 0.0f, 1.0f, 1.0f});
    setColor(defaultNormalTexture, Color4{0.5f, 0.5f, 1.0f, 1.0f});
    setColor(defaultEmissiveTexture, Color4{0.0f, 0.0f, 0.0f, 1.0f});
    setColor(defaultMetallicRoughnessTexture, Color4{0.0f, 0.5f, 0.5f, 1.0f});
    setColor(defaultAmbientOcclusionTexture, Color4{1.0f, 1.0f, 1.0f, 1.0f});
    setColorCubemap(defaultSkyboxTexture, Color4{0.5f, 0.5f, 0.5f, 1.0f});

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
        shaders.fxaa = ShaderFXAA(config);
        shaders.pointCloud = ShaderPointCloud(config);
        shaders.lines = ShaderLines(config);
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
        gBuffer.fboObjectId.setStorage(0, viewport, PixelType::Rg8u);

        if (!gBuffer.fboInit) {
            gBuffer.fboInit = true;
            gBuffer.fbo.attach(gBuffer.fboColorAlpha, FramebufferAttachment::Color0, 0);
            gBuffer.fbo.attach(gBuffer.fboEmissive, FramebufferAttachment::Color1, 0);
            gBuffer.fbo.attach(gBuffer.fboMetallicRoughnessAmbient, FramebufferAttachment::Color2, 0);
            gBuffer.fbo.attach(gBuffer.fboNormal, FramebufferAttachment::Color3, 0);
            gBuffer.fbo.attach(gBuffer.fboObjectId, FramebufferAttachment::Color4, 0);
            gBuffer.fbo.attach(gBuffer.fboDepth, FramebufferAttachment::DepthStencil, 0);
        }

        gBuffer.size = viewport;
    }

    if (postProcessing.size != viewport) {
        postProcessing.fboTexture[0].setStorage(0, viewport, PixelType::Rgb8u);
        postProcessing.fboTexture[1].setStorage(0, viewport, PixelType::Rgb8u);
        postProcessing.fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);

        if (!postProcessing.fboInit) {
            postProcessing.fboInit = true;

            postProcessing.fbo[0].attach(postProcessing.fboTexture[0], FramebufferAttachment::Color0, 0);
            postProcessing.fbo[0].attach(postProcessing.fboDepth, FramebufferAttachment::DepthStencil, 0);
            postProcessing.fbo[1].attach(postProcessing.fboTexture[1], FramebufferAttachment::Color0, 0);
            postProcessing.fbo[1].attach(postProcessing.fboDepth, FramebufferAttachment::DepthStencil, 0);
        }

        postProcessing.size = viewport;
    }

    Framebuffer::DefaultFramebuffer.bind();

    updateLights(scene);
    updateCameras(viewport, scene);
    updateBullets(scene);

    gBuffer.fbo.bind();
    renderScenePbr(viewport, scene);

    // blit(viewport, gBuffer.fbo, Framebuffer::DefaultFramebuffer, FramebufferAttachment::Color3);

    postProcessing.fbo[postProcessing.inputIdx].bind();
    renderSceneBackground(viewport, scene);
    renderPbr();
    renderSceneForward(viewport, scene);

    applyFxaa(viewport);

    blit(viewport, postProcessing.fbo[postProcessing.inputIdx], Framebuffer::DefaultFramebuffer,
         FramebufferAttachment::Color0, BufferBit::Color);

    Framebuffer::DefaultFramebuffer.bind();
    renderCanvas(viewport, scene);
}

void Renderer::applyFxaa(const Vector2i& viewport) {
    postProcessing.fbo[postProcessing.outputIdx].bind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    shaders.fxaa.use();
    shaders.fxaa.setInputSize(viewport);
    shaders.fxaa.setOutputSize(viewport);
    shaders.fxaa.setTextureSize(viewport);
    shaders.fxaa.setFrameCount(0);
    shaders.fxaa.setFrameDirection(0);
    shaders.fxaa.setMvpMatrix(Matrix4{1.0f});
    shaders.fxaa.bindTexture(postProcessing.fboTexture[postProcessing.inputIdx]);
    shaders.fxaa.draw(fullScreenMesh);

    std::swap(postProcessing.inputIdx, postProcessing.outputIdx);
}

void Renderer::blit(const Vector2i& viewport, Framebuffer& source, Framebuffer& target,
                    const FramebufferAttachment attachment, BufferBit bufferBit) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, source.getHandle());
    glReadBuffer(GLuint(attachment));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getHandle());
    glViewport(0, 0, viewport.x, viewport.y);
    glBlitFramebuffer(0, 0, viewport.x, viewport.y, 0, 0, viewport.x, viewport.y, GLenum(bufferBit), GL_NEAREST);
}

EntityPtr Renderer::queryEntityAtPos(Scene& scene, const Vector2i& pos) {
    if (pos.x >= gBuffer.size.x || pos.y >= gBuffer.size.y || pos.x < 0 || pos.y < 0) {
        return nullptr;
    }

    std::unique_ptr<char[]> pixels(new char[gBuffer.size.x * gBuffer.size.y * 2]);
    gBuffer.fboObjectId.getPixels(0, PixelType::Rg8u, pixels.get());

    ObjectId pixel;
    pixel.data[0] = pixels.get()[(gBuffer.size.y - pos.y - 1) * gBuffer.size.x * 2 + pos.x * 2 + 0];
    pixel.data[1] = pixels.get()[(gBuffer.size.y - pos.y - 1) * gBuffer.size.x * 2 + pos.x * 2 + 1];

    return scene.getEntityById(pixel.id);
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

    auto& componentSystemCanvasLines = scene.getComponentSystem<ComponentLines>();
    for (auto& component : componentSystemCanvasLines) {
        renderComponentCanvasLines(*camera, *component);
    }*/

/*auto& componentSystemCanvasImage = scene.getComponentSystem<ComponentPointCloud>();
for (auto& component : componentSystemCanvasImage) {
    renderComponentCanvasImage(*camera, *component);
}

auto& componentSystemText = scene.getComponentSystem<ComponentText>();
for (auto& component : componentSystemText) {
    renderComponentText(*camera, *component);
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
if (skybox.textures.irradiance) {
    shaders.pbr.bindSkyboxIrradianceTexture(skybox.textures.irradiance);
} else {
    shaders.pbr.bindSkyboxIrradianceTexture(defaultSkyboxTexture);
}
if (skybox.textures.prefilter) {
    shaders.pbr.bindSkyboxPrefilterTexture(skybox.textures.prefilter);
} else {
    shaders.pbr.bindSkyboxIrradianceTexture(defaultSkyboxTexture);
}
shaders.pbr.bindCameraUniform(cameraUbo);
shaders.pbr.bindDirectionalLightsUniform(directionalLightsUbo);
shaders.pbr.bindSSAO(ssaoUbo);
shaders.pbr.draw(fullScreenMesh);
}

void Renderer::renderSceneBackground(const Vector2i& viewport, Scene& scene) {
glViewport(0, 0, viewport.x, viewport.y);
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
} else {
    renderSkybox(glm::scale(Matrix4{1.0f}, Vector3{100.0f}), defaultSkyboxTexture);
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

blit(viewport, gBuffer.fbo, postProcessing.fbo[postProcessing.inputIdx], FramebufferAttachment::DepthStencil,
     BufferBit::Depth);

glDisable(GL_BLEND);
glDisable(GL_DEPTH_TEST);
glDepthMask(GL_FALSE);
glBlendFunc(GL_SRC_ALPHA, GL_ONE);

auto& componentSystemLines = scene.getComponentSystem<ComponentLines>();

shaders.lines.use();
shaders.lines.bindCameraUniform(cameraUbo);
auto primary = getPrimaryCamera(scene);

for (auto& component : componentSystemLines) {
    renderComponentLines(*component);
}

auto& componentSystemPointCloud = scene.getComponentSystem<ComponentPointCloud>();

shaders.pointCloud.use();
shaders.pointCloud.bindCameraUniform(cameraUbo);

for (auto& component : componentSystemPointCloud) {
    renderComponentPointCloud(*component);
}

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
auto& componentSystemPlanet = scene.getComponentSystem<ComponentPlanet>();

GLuint attachments[5] = {
    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4,
};
glDrawBuffers(5, attachments);

glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
glDisable(GL_BLEND);

glViewport(0, 0, viewport.x, viewport.y);
static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
glClearBufferfv(GL_COLOR, 0, &black.x);
glClearBufferfv(GL_COLOR, 4, &black.x);
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

if (data.empty()) {
    return;
}

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
renderSkybox(component.getObject().getTransform(), skybox.textures.texture);
}

void Renderer::renderSkybox(const Matrix4& modelMatrix, const TextureCubemap& cubemap) {
shaders.skybox.setModelMatrix(modelMatrix);
shaders.skybox.bindSkyboxTexture(cubemap);
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

const auto transform = component.getObject().getAbsoluteTransform();
shaders.particleEmitter.setModelMatrix(transform);
shaders.particleEmitter.setModelViewMatrix(cameraViewMatrix * transform);
shaders.particleEmitter.bindParticleTexture(component.getTexture()->getTexture());
shaders.particleEmitter.bindParticleData(component.getUbo());
particleEmitter.vao.bind();
glDrawArrays(GL_POINTS, 0, component.getCount());
}

void Renderer::renderComponentPointCloud(ComponentPointCloud& component) {
component.recalculate();

const auto transform = component.getObject().getAbsoluteTransform();
shaders.pointCloud.setModelMatrix(transform);
shaders.pointCloud.bindTexture(component.getTexture()->getTexture());
shaders.pointCloud.draw(component.getMesh());
}

void Renderer::renderComponentLines(ComponentLines& component) {
component.recalculate();

const auto transform = component.getObject().getAbsoluteTransform();
shaders.lines.setModelMatrix(transform);
shaders.lines.draw(component.getMesh());
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
const auto transform = component.getObject().getAbsoluteTransform();
if (auto entity = component.getObject().asEntity()) {
    shaders.model.setObjectId(entityIdToObjectIdColor(entity));
}
renderModel(component.getModel(), transform);
}

void Renderer::renderComponentTurret(ComponentTurret& component) {
const auto& turret = component.getTurret()->getComponents();
const auto absoluteTransform = component.getObject().getAbsoluteTransform();
const auto transformInverted = glm::inverse(absoluteTransform);

auto transform = glm::translate(absoluteTransform, turret.base.offset);
renderModel(turret.base.model, transform);

const auto& rotation = component.getRotation();

transform = glm::translate(absoluteTransform, turret.arm.offset);
transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
renderModel(turret.arm.model, transform);

transform = glm::translate(absoluteTransform, turret.cannon.offset);
transform = glm::rotate(transform, rotation.y, Vector3{0.0f, 1.0f, 0.0f});
transform = glm::rotate(transform, rotation.x, Vector3{1.0f, 0.0f, 0.0f});

renderModel(turret.cannon.model, transform);
}

void Renderer::renderComponentGrid(ComponentGrid& component) {
component.rebuild();

const auto transform = component.getObject().getAbsoluteTransform();
shaders.grid.setModelMatrix(transform);

if (auto entity = component.getObject().asEntity()) {
    shaders.grid.setObjectId(entityIdToObjectIdColor(entity));
}

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

void Renderer::renderComponentText(const Camera& camera, ComponentText& component) {
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
}*/