#include "RenderPassSkyboxColor.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassSkyboxColor::RenderPassSkyboxColor(VulkanRenderer& vulkan, RenderBufferSkybox& buffer,
                                             RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassSkybox"},
    vulkan{vulkan},
    resources{resources},
    pipelineSkyboxNebula{vulkan},
    pipelineSkyboxStars{vulkan} {

    { // Color
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 1.0f};
        addAttachment(RenderBufferSkybox::Attachment::Color, attachment);
    }

    addSubpass(
        {
            RenderBufferSkybox::Attachment::Color,
        },
        {});

    addPipeline(pipelineSkyboxNebula, 0);
    addPipeline(pipelineSkyboxStars, 0);
}

void RenderPassSkyboxColor::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassSkyboxColor::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderNebula(vkb, scene);
    renderPointClouds(vkb, scene);
}

void RenderPassSkyboxColor::renderNebula(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto& camera = *scene.getPrimaryCamera();
    const auto systemNebulas = scene.getView<ComponentNebula>();

    pipelineSkyboxNebula.bind(vkb);

    pipelineSkyboxNebula.setUniformCamera(camera.getUbo().getCurrentBuffer());

    for (auto&& [entity, nebula] : systemNebulas.each()) {
        nebula.recalculate(vulkan);

        pipelineSkyboxNebula.setUniformNebula(nebula.getUbo());
        pipelineSkyboxNebula.flushDescriptors(vkb);

        pipelineSkyboxNebula.renderMesh(vkb, resources.getMeshSkyboxCube());
    }
}

void RenderPassSkyboxColor::renderPointClouds(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto& camera = *scene.getPrimaryCamera();
    const auto systemPointClouds = scene.getView<ComponentTransform, ComponentPointCloud>();

    pipelineSkyboxStars.bind(vkb);

    pipelineSkyboxStars.setUniformCamera(camera.getUbo().getCurrentBuffer());

    for (auto&& [entity, transform, pointCloud] : systemPointClouds.each()) {
        pointCloud.recalculate(vulkan);

        const auto& mesh = pointCloud.getMesh();
        if (mesh.count == 0) {
            continue;
        }

        pipelineSkyboxStars.setTextureColor(pointCloud.getTexture());
        pipelineSkyboxStars.flushDescriptors(vkb);

        const auto modelMatrix = transform.getAbsoluteTransform();
        pipelineSkyboxStars.setModelMatrix(modelMatrix);
        pipelineSkyboxStars.flushConstants(vkb);

        pipelineSkyboxStars.renderMesh(vkb, mesh);
    }
}
