#include "Renderer.hpp"

#include "../Assets/Model.hpp"
#include "../Graphics/BrdfRenderer.hpp"
#include "../Graphics/Skybox.hpp"
#include "../Scene/ComponentGrid.hpp"
#include "../Scene/ComponentLines.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/ComponentPointCloud.hpp"
#include "../Scene/ComponentWireframe.hpp"
#include "../Scene/Scene.hpp"
#include "Primitives.hpp"

using namespace Scissio;

static const Matrix4 SkyboxModelMatrix = glm::scale(Matrix4{1.0f}, Vector3{100.0f});

Renderer::Renderer(const Config& config)
    : shaderModel(config), shaderGrid(config), shaderGBufferView(config), shaderPbr(config), shaderSkybox(config),
      shaderPointCloud(config), shaderLines(config),
      shaderWireframe(config), brdf{NO_CREATE}, viewport{1920.0f, 1080.0f}, skybox{nullptr}, gbuffer{nullptr},
      fullScreenQuad{NO_CREATE}, skyboxMesh{NO_CREATE} {

    brdf = BrdfRenderer(config).render();

    fullScreenQuad = createFullScreenMesh();
    skyboxMesh = createSkyboxMesh();

    setView(glm::lookAt(glm::vec3{3.0f, 3.0f, 3.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}));
}

void Renderer::setView(const glm::mat4x4& viewMatrix) {
    this->viewMatrix = viewMatrix;
    projectionViewMatrix = projectionMatrix * viewMatrix;
}

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

void Renderer::setSkybox(const Skybox& skybox) {
    this->skybox = &skybox;
}

void Renderer::setGBuffer(GBuffer& gbuffer) {
    this->gbuffer = &gbuffer;
}

void Renderer::setFBuffer(FBuffer& fbuffer) {
    this->fbuffer = &fbuffer;
}

void Renderer::setQueryPos(const Vector2i& queryPos) {
    this->queryPos = queryPos;
}

std::optional<Object*> Renderer::render(Scene& scene, const RenderOptions& options) {
    assert(!!skybox);
    assert(!!gbuffer);

    Object* queryResult = nullptr;

    auto& systemModel = scene.getComponentSystem<ComponentModel>();
    auto& systemGrid = scene.getComponentSystem<ComponentGrid>();
    auto& systemPointCloud = scene.getComponentSystem<ComponentPointCloud>();
    auto& systemLines = scene.getComponentSystem<ComponentLines>();
    auto& systemWireframe = scene.getComponentSystem<ComponentWireframe>();

    std::vector<Object*> objectIdMap;

    gbuffer->bind(viewport);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);

    const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);

    // ComponentModel
    {
        shaderModel.use();
        shaderModel.setTransformationProjectionMatrix(projectionViewMatrix);
        for (const auto& component : systemModel) {
            objectIdMap.push_back(&component->getObject());
            shaderModel.setObjectId(static_cast<uint16_t>(objectIdMap.size()));
            component->render(shaderModel);
        }
    }

    // ComponentGrid
    {
        shaderGrid.use();
        shaderGrid.setTransformationProjectionMatrix(projectionViewMatrix);
        for (const auto& component : systemGrid) {
            objectIdMap.push_back(&component->getObject());
            shaderGrid.setObjectId(static_cast<uint16_t>(objectIdMap.size()));
            component->render(shaderGrid);
            component->clearDirty();
        }
    }

    const auto objectIdIndex = gbuffer->getObjectIdAtPosition(queryPos);
    if (objectIdIndex > 0 && objectIdIndex <= objectIdMap.size()) {
        queryResult = objectIdMap.at(objectIdIndex - 1);
    }

    glDisable(GL_DEPTH_TEST);
    gbuffer->unbind();

    fbuffer->bind(viewport);
    gbuffer->copyDepth(*fbuffer);

    static Color4 color{0.0f, 0.0f, 0.0f, 1.0f};
    glClearBufferfv(GL_COLOR, 0, &color.x);

    // Skybox
    if (options.withSkybox) {
        auto cameraMatrix = projectionViewMatrix;
        cameraMatrix[3] = Vector4{0.0f, 0.0f, 0.0f, 1.0f};

        shaderSkybox.use();
        shaderSkybox.setTransformationProjectionMatrix(cameraMatrix);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

        shaderSkybox.bindSkyboxTexture(skybox->texture);
        shaderSkybox.setModelMatrix(SkyboxModelMatrix);

        shaderSkybox.draw(skyboxMesh);
    }

    renderGBuffer();

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // ComponentWireframe
    {
        shaderWireframe.use();
        shaderWireframe.setTransformationProjectionMatrix(projectionViewMatrix);
        shaderWireframe.setEyesPos(eyesPos);
        for (const auto& component : systemWireframe) {
            objectIdMap.push_back(&component->getObject());
            component->render(shaderWireframe);
        }
    }

    glDisable(GL_DEPTH_TEST);

    // ComponentPointCloud
    {
        shaderPointCloud.use();
        shaderPointCloud.setProjectionMatrix(projectionMatrix);
        shaderPointCloud.setViewMatrix(viewMatrix);
        for (const auto& component : systemPointCloud) {
            component->render(shaderPointCloud);
        }
    }

    // ComponentLines
    {
        shaderLines.use();
        shaderLines.setProjectionViewMatrix(projectionViewMatrix);
        for (const auto& component : systemLines) {
            component->render(shaderLines);
        }
    }

    glDisable(GL_BLEND);
    fbuffer->unbind();

    // TODO: Postprocessing

    return queryResult ? std::make_optional(queryResult) : std::nullopt;
}

void Renderer::renderGBuffer() {
    assert(!!gbuffer);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

    const auto eyesPos = Vector3(glm::inverse(viewMatrix)[3]);
    const auto projectionViewInverseMatrix = glm::inverse(projectionViewMatrix);

    shaderPbr.use();
    shaderPbr.bindDepthTexture(gbuffer->getDepth());
    shaderPbr.bindBaseColorRoughnessTexture(gbuffer->getColorRoughness());
    shaderPbr.bindEmissiveAmbientTexture(gbuffer->getEmissiveMetallic());
    shaderPbr.bindNormalMetallicTexture(gbuffer->getNormalAmbient());
    shaderPbr.bindBrdfTexture(brdf);
    shaderPbr.bindSkyboxIrradianceTexture(skybox->irradiance);
    shaderPbr.bindSkyboxPrefilterTexture(skybox->prefilter);
    shaderPbr.setViewport(viewport);
    shaderPbr.setViewProjectionInverseMatrix(projectionViewInverseMatrix);
    shaderPbr.setEyesPos(eyesPos);

    shaderPbr.draw(fullScreenQuad);
}
