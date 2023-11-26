#include "RenderPassSkybox.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Controllers/ControllerLights.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassSkybox::RenderPassSkybox(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                   RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassSkybox"},
    resources{resources},
    pipelinePlanet{vulkan, assetsManager},
    pipelineSkybox{vulkan, assetsManager},
    pipelineStarFlare{vulkan, assetsManager} {

    { // Depth
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.clearColor.depthStencil = {1.0f, 0x00};
        addAttachment(RenderBufferPbr::Attachment::Depth, attachment);
    }

    { // Forward
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
        addAttachment(RenderBufferPbr::Attachment::Forward, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::Depth,
            RenderBufferPbr::Attachment::Forward,
        },
        {});

    addPipeline(pipelinePlanet, 0);
    addPipeline(pipelineSkybox, 0);
    addPipeline(pipelineStarFlare, 0);

    textureBrdf = assetsManager.getTextures().find("brdf");
}

void RenderPassSkybox::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassSkybox::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    const auto* skybox = scene.getSkybox();
    const auto* skyboxTextures = &resources.getDefaultSkybox();
    if (skybox && skybox->isGenerated() && skybox->getTextures().getTexture()) {
        skyboxTextures = &skybox->getTextures();
    }

    renderSkybox(vkb, scene, *skyboxTextures);
    renderPlanets(vkb, scene, *skyboxTextures);
    renderStarFlare(vkb, scene);
}

void RenderPassSkybox::renderSkybox(VulkanCommandBuffer& vkb, Scene& scene, const SkyboxTextures& skyboxTextures) {
    const auto& camera = *scene.getPrimaryCamera();

    pipelineSkybox.bind(vkb);

    const auto modelMatrix = glm::scale(Matrix4{1.0f}, Vector3{1000.0f});
    pipelineSkybox.setModelMatrix(modelMatrix);
    pipelineSkybox.flushConstants(vkb);
    pipelineSkybox.setUniformCamera(camera.getUboZeroPos().getCurrentBuffer());
    pipelineSkybox.setTextureBaseColor(skyboxTextures.getTexture());
    pipelineSkybox.flushDescriptors(vkb);
    pipelineSkybox.renderMesh(vkb, resources.getMeshSkyboxCube());
}

void RenderPassSkybox::renderPlanets(VulkanCommandBuffer& vkb, Scene& scene, const SkyboxTextures& skyboxTextures) {
    const auto& camera = *scene.getPrimaryCamera();
    const auto& controllerLights = scene.getController<ControllerLights>();

    pipelinePlanet.bind(vkb);

    const auto& entities = scene.getView<ComponentTransform, ComponentPlanet>(entt::exclude<TagDisabled>).each();
    for (auto&& [_, transform, component] : entities) {
        if (!component.isBackground()) {
            continue;
        }

        const PlanetTextures* planetTextures;

        if (component.isHighRes()) {
            if (!component.getTextures().getColor()) {
                continue;
            }

            planetTextures = &component.getTextures();
        } else {
            planetTextures = &component.getPlanetType()->getLowResTextures();
        }

        const auto modelMatrix = transform.getAbsoluteTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));
        pipelinePlanet.setModelMatrix(modelMatrix);
        pipelinePlanet.setNormalMatrix(normalMatrix);
        pipelinePlanet.flushConstants(vkb);

        pipelinePlanet.setUniformCamera(camera.getUboZeroPos().getCurrentBuffer());
        pipelinePlanet.setUniformDirectionalLights(controllerLights.getUboDirectionalLights().getCurrentBuffer());
        pipelinePlanet.setUniformAtmosphere(component.getPlanetType()->getUbo());
        pipelinePlanet.setTextureAlbedo(planetTextures->getColor());
        pipelinePlanet.setTextureNormal(planetTextures->getNormal());
        pipelinePlanet.setTextureMetallicRoughness(planetTextures->getMetallicRoughness());
        pipelinePlanet.setTextureSkyboxIrradiance(skyboxTextures.getIrradiance());
        pipelinePlanet.setTextureSkyboxPrefilter(skyboxTextures.getPrefilter());
        pipelinePlanet.setTextureBrdf(textureBrdf->getVulkanTexture());
        pipelinePlanet.flushDescriptors(vkb);

        pipelinePlanet.renderMesh(vkb, resources.getMeshPlanet());
    }
}

void RenderPassSkybox::renderStarFlare(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto& camera = *scene.getPrimaryCamera();

    pipelineStarFlare.bind(vkb);

    const auto& entities = scene.getView<ComponentTransform, ComponentStarFlare>(entt::exclude<TagDisabled>).each();
    for (auto&& [_, transform, component] : entities) {
        const auto& mesh = component.getMesh();
        if (mesh.count == 0) {
            return;
        }

        const auto modelMatrix = transform.getAbsoluteTransform();
        pipelineStarFlare.setModelMatrix(modelMatrix);
        pipelineStarFlare.setSize(component.getSize());
        pipelineStarFlare.setTemp(component.getTemperature());
        pipelineStarFlare.flushConstants(vkb);

        pipelineStarFlare.setUniformCamera(camera.getUboZeroPos().getCurrentBuffer());
        pipelineStarFlare.setTextureColor(component.getTexture()->getVulkanTexture());
        pipelineStarFlare.setTextureSpectrumLow(component.getTextureLow()->getVulkanTexture());
        pipelineStarFlare.setTextureSpectrumHigh(component.getTextureHigh()->getVulkanTexture());
        pipelineStarFlare.flushDescriptors(vkb);

        pipelineStarFlare.renderMesh(vkb, mesh);
    }
}
