#include "Renderer.hpp"

#include "../Assets/Model.hpp"
#include "../Graphics/BrdfRenderer.hpp"
#include "../Graphics/Skybox.hpp"

using namespace Scissio;

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

static const float FULL_SCREEN_QUAD[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

Renderer::Renderer(const Config& config)
    : config(config), shaderModel(config), shaderGrid(config), shaderGBufferView(config), shaderPbr(config),
      shaderSkybox(config), brdf(NO_CREATE) {
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

    setColor(defaultBaseColor, Color4{1.0f, 0.0f, 1.0f, 1.0f});
    setColor(defaultNormal, Color4{0.5f, 0.5f, 1.0f, 1.0f});
    setColor(defaultEmissive, Color4{0.0f, 0.0f, 0.0f, 1.0f});
    setColor(defaultMetallicRoughness, Color4{0.0f, 0.5f, 0.5f, 1.0f});
    setColor(defaultAmbientOcclusion, Color4{1.0f, 1.0f, 1.0f, 1.0f});

    VertexBuffer vbo(VertexBufferType::Array);
    vbo.bufferData(FULL_SCREEN_QUAD, sizeof(FULL_SCREEN_QUAD), VertexBufferUsage::StaticDraw);

    fullScreenQuad.addVertexBuffer(std::move(vbo), ShaderGBufferView::Position{});
    fullScreenQuad.setCount(6);
    fullScreenQuad.setPrimitive(PrimitiveType::Triangles);

    vbo = VertexBuffer(VertexBufferType::Array);
    vbo.bufferData(SKYBOX_VERTICES, sizeof(SKYBOX_VERTICES), VertexBufferUsage::StaticDraw);

    skyboxMesh.addVertexBuffer(std::move(vbo), ShaderSkybox::Position{});
    skyboxMesh.setCount(6 * 2 * 3);
    skyboxMesh.setPrimitive(PrimitiveType::Triangles);

    brdf = BrdfRenderer(config).render();

    setCamera(glm::lookAt(glm::vec3{3.0f, 3.0f, 3.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}));
}

/*void Renderer::beginFrame(const Vector2i& viewport, GBuffer& gBuffer) {
    this->viewport = viewport;
    if (viewport != viewportOld) {
        // |   color.r   |   color.g   |   color.b   |   roughness   |
        // |  emissive.r |  emissive.g |  emissive.b |   metallic    |
        // |  normal.x   |  normal.y   |  normal.z   |   ambient     |
        // |    pos.x    |    pos.y    |    pos.z    |      --       |

        fboDepth.setStorage(0, viewport, PixelType::Depth24Stencil8);
        fboColorRoughness.setStorage(0, viewport, PixelType::Rgba8u);
        fboEmissiveMetallic.setStorage(0, viewport, PixelType::Rgba8u);
        fboNormalAmbient.setStorage(0, viewport, PixelType::Rgba16f);

        fbo.attach(fboColorRoughness, FramebufferAttachment::Color0, 0);
        fbo.attach(fboEmissiveMetallic, FramebufferAttachment::Color1, 0);
        fbo.attach(fboNormalAmbient, FramebufferAttachment::Color2, 0);
        fbo.attach(fboDepth, FramebufferAttachment::DepthStencil, 0);

        viewportOld = viewport;
    }

    fbo.bind();

    GLuint attachments[4] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    };
    glDrawBuffers(4, attachments);

    glViewport(0, 0, viewport.x, viewport.y);
    // static Color4 black{0.0f, 0.0f, 0.0f, 0.0f};
    // glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    gBuffer.bind(viewport);

    glDisable(GL_BLEND);

    projectionMatrix =
        glm::perspective(glm::radians(70.0f), viewport.x / static_cast<float>(viewport.y), 0.1f, 1000.0f);

    setCamera(glm::lookAt(glm::vec3{3.0f, 3.0f, 3.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}));
}*/

/*void Renderer::endFrame() {
    Framebuffer::DefaultFramebuffer.bind();

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);
}*/

/*void Renderer::blit(const Skybox& skybox, const GBuffer& gBuffer) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    renderSkybox(skybox.texture);
    renderPbr(skybox, gBuffer);
}*/

/*void Renderer::endFrameDebug(const int mode) {
    Framebuffer::DefaultFramebuffer.bind();

    GLuint attachments[1] = {
        GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(1, attachments);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    static Color4 black{0.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, &black.x);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);

    renderGBuffer(mode);
}*/

void Renderer::setViewport(const Vector2i& viewport) {
    this->viewport = viewport;
}

void Renderer::setProjection(const float fov) {
    projectionMatrix = glm::perspective(glm::radians(fov), viewport.x / static_cast<float>(viewport.y), 0.1f, 1000.0f);
    projectionViewMatrix = projectionMatrix * viewMatrix;
}

void Renderer::setProjection(const Matrix4& projectionMatrix) {
    this->projectionMatrix = projectionMatrix;
    projectionViewMatrix = projectionMatrix * viewMatrix;
}

void Renderer::renderSkybox(const TextureCubemap& texture) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    static const Matrix4 SKYBOX_MODEL_MATRIX = glm::scale(Matrix4{1.0f}, Vector3{100.0f});
    // static const Matrix4 SKYBOX_MODEL_MATRIX = Matrix4{1.0f};

    shaderSkybox.use();

    auto cameraMatrix = projectionViewMatrix;
    cameraMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
    shaderSkybox.bindSkyboxTexture(texture);
    shaderSkybox.setModelMatrix(SKYBOX_MODEL_MATRIX);
    shaderSkybox.setTransformationProjectionMatrix(cameraMatrix);

    shaderSkybox.draw(skyboxMesh);
}

void Renderer::renderPbr(const GBuffer& gBuffer, const Skybox& skybox) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);
    const auto projectionViewInverseMatrix = glm::inverse(projectionViewMatrix);

    shaderPbr.use();
    shaderPbr.bindDepthTexture(gBuffer.getDepth());
    shaderPbr.bindBaseColorRoughnessTexture(gBuffer.getColorRoughness());
    shaderPbr.bindEmissiveAmbientTexture(gBuffer.getEmissiveMetallic());
    shaderPbr.bindNormalMetallicTexture(gBuffer.getNormalAmbient());
    shaderPbr.bindBrdfTexture(brdf);
    shaderPbr.bindSkyboxIrradianceTexture(skybox.irradiance);
    shaderPbr.bindSkyboxPrefilterTexture(skybox.prefilter);
    shaderPbr.setViewport(viewport);
    shaderPbr.setViewProjectionInverseMatrix(projectionViewInverseMatrix);
    shaderPbr.setEyesPos(eyesPos);

    shaderPbr.draw(fullScreenQuad);
}

/*void Renderer::renderGBuffer(const int mode) {
    shaderGBufferView.use();
    shaderGBufferView.bindDepthTexture(fboDepth);
    shaderGBufferView.bindBaseColorRoughnessTexture(fboColorRoughness);
    shaderGBufferView.bindEmissiveAmbientTexture(fboEmissiveMetallic);
    shaderGBufferView.bindNormalMetallicTexture(fboNormalAmbient);
    shaderGBufferView.setViewport(viewport);
    shaderGBufferView.setMode(mode);

    shaderGBufferView.draw(fullScreenQuad);
}*/

void Renderer::setCamera(const glm::mat4x4& viewMatrix) {
    this->viewMatrix = viewMatrix;
    projectionViewMatrix = projectionMatrix * viewMatrix;
}

void Renderer::renderGridModel(const std::list<Primitive>& primitives, const Matrix4& transform) {
    shaderGrid.use();

    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));

    shaderGrid.setModelMatrix(transform);
    // shaderGrid.setNormalMatrix(transformInverted);
    shaderGrid.setTransformationProjectionMatrix(projectionViewMatrix);

    for (const auto& primitive : primitives) {
        // shaderModel.bindBaseColorTexture(primitive.)
        const auto& material = primitive.material;
        if (material.baseColorTexture) {
            shaderGrid.bindBaseColorTexture(material.baseColorTexture->getTexture());
        } else {
            shaderGrid.bindBaseColorTexture(defaultBaseColor);
        }

        if (material.emissiveTexture) {
            shaderGrid.bindEmissiveTexture(material.emissiveTexture->getTexture());
        } else {
            shaderGrid.bindEmissiveTexture(defaultEmissive);
        }

        if (material.normalTexture) {
            shaderGrid.bindNormalTexture(material.normalTexture->getTexture());
        } else {
            shaderGrid.bindNormalTexture(defaultNormal);
        }

        if (material.metallicRoughnessTexture) {
            shaderGrid.bindMetallicRoughnessTexture(material.metallicRoughnessTexture->getTexture());
        } else {
            shaderGrid.bindMetallicRoughnessTexture(defaultMetallicRoughness);
        }

        if (material.ambientOcclusionTexture) {
            shaderGrid.bindAmbientOcclusionTexture(material.ambientOcclusionTexture->getTexture());
        } else {
            shaderGrid.bindAmbientOcclusionTexture(defaultAmbientOcclusion);
        }

        shaderGrid.draw(primitive.mesh);
    }
}

void Renderer::renderModel(const Model& model, const glm::mat4x4& transform) {
    shaderModel.use();

    const auto transformInverted = glm::transpose(glm::inverse(glm::mat3x3(transform)));

    shaderModel.setModelMatrix(transform);
    shaderModel.setNormalMatrix(transformInverted);
    shaderModel.setTransformationProjectionMatrix(projectionViewMatrix);

    for (const auto& primitive : model.getPrimitives()) {
        // shaderModel.bindBaseColorTexture(primitive.)
        const auto& material = primitive.material;
        if (material.baseColorTexture) {
            shaderModel.bindBaseColorTexture(material.baseColorTexture->getTexture());
        } else {
            shaderModel.bindBaseColorTexture(defaultBaseColor);
        }

        if (material.emissiveTexture) {
            shaderModel.bindEmissiveTexture(material.emissiveTexture->getTexture());
        } else {
            shaderModel.bindEmissiveTexture(defaultEmissive);
        }

        if (material.normalTexture) {
            shaderModel.bindNormalTexture(material.normalTexture->getTexture());
        } else {
            shaderModel.bindNormalTexture(defaultNormal);
        }

        if (material.metallicRoughnessTexture) {
            shaderModel.bindMetallicRoughnessTexture(material.metallicRoughnessTexture->getTexture());
        } else {
            shaderModel.bindMetallicRoughnessTexture(defaultMetallicRoughness);
        }

        if (material.ambientOcclusionTexture) {
            shaderModel.bindAmbientOcclusionTexture(material.ambientOcclusionTexture->getTexture());
        } else {
            shaderModel.bindAmbientOcclusionTexture(defaultAmbientOcclusion);
        }

        shaderModel.draw(primitive.mesh);
    }
}
