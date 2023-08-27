#include "render_pass_shadow.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/controllers/controller_lights.hpp"
#include "../../scene/controllers/controller_static_model.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassShadow::RenderPassShadow(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                   RenderResources& resources, AssetsManager& assetsManager, const uint32_t index) :
    RenderPass{vulkan, buffer, "RenderPassShadow"},
    resources{resources},
    index{index},
    pipelineGrid{vulkan, assetsManager},
    pipelineModel{vulkan, assetsManager},
    pipelineModelInstanced{vulkan, assetsManager} {

    { // Depth
        AttachmentInfo attachmentInfo{};
        attachmentInfo.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentInfo.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentInfo.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachmentInfo.clearColor.depthStencil = {1.0f, 0x00};
        addAttachment(RenderBufferPbr::Attachment::ShadowL0 + index, attachmentInfo);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::ShadowL0 + index,
        },
        {});

    addPipeline(pipelineGrid, 0);
    addPipeline(pipelineModel, 0);
    addPipeline(pipelineModelInstanced, 0);
}

void RenderPassShadow::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassShadow::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderGrids(vkb, scene);
    renderModels(vkb, scene);
    renderModelsInstanced(vkb, scene);
}

void RenderPassShadow::renderGrids(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>(entt::exclude<TagDisabled>);

    pipelineGrid.bind(vkb);

    for (auto&& [entity, transform, grid] : systemGrids.each()) {
        const auto modelMatrix = transform.getAbsoluteTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineGrid.setModelMatrix(modelMatrix);
        pipelineGrid.setNormalMatrix(normalMatrix);
        pipelineGrid.setEntityColor(entityColor(entity));
        pipelineGrid.flushConstants(vkb);

        for (auto& primitive : grid.getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            pipelineGrid.setUniformCamera(controllerLights.getUboShadowCamera().getCurrentBuffer(), index);
            pipelineGrid.flushDescriptors(vkb);

            pipelineGrid.renderMesh(vkb, primitive.mesh);
        }
    }
}

void RenderPassShadow::renderModels(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto systemModels = scene.getView<ComponentTransform, ComponentModel>(entt::exclude<TagDisabled>);

    pipelineModel.bind(vkb);

    for (auto&& [entity, transform, model] : systemModels.each()) {
        if (transform.isStatic()) {
            continue;
        }

        const auto modelMatrix = transform.getAbsoluteTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineModel.setModelMatrix(modelMatrix);
        pipelineModel.setNormalMatrix(normalMatrix);
        pipelineModel.setEntityColor(entityColor(entity));
        pipelineModel.flushConstants(vkb);

        for (const auto& node : model.getModel()->getNodes()) {
            // Skip animated models
            if (node.skin) {
                continue;
            }

            for (auto& primitive : node.primitives) {
                if (!primitive.material) {
                    EXCEPTION("Primitive has no material");
                }

                pipelineModel.setUniformCamera(controllerLights.getUboShadowCamera().getCurrentBuffer(), index);
                pipelineModel.flushDescriptors(vkb);

                pipelineModel.renderMesh(vkb, primitive.mesh);
            }
        }
    }
}

void RenderPassShadow::renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerLights = scene.getController<ControllerLights>();
    auto& controllerStaticModel = scene.getController<ControllerStaticModel>();

    pipelineModelInstanced.bind(vkb);

    for (auto&& [model, buffer] : controllerStaticModel.getBuffers()) {
        for (const auto& node : model->getNodes()) {
            // Skip animated models
            if (node.skin) {
                continue;
            }
            
            for (auto& primitive : node.primitives) {
                pipelineModelInstanced.setUniformCamera(controllerLights.getUboShadowCamera().getCurrentBuffer(),
                                                        index);
                pipelineModelInstanced.flushDescriptors(vkb);

                pipelineModelInstanced.renderMeshInstanced(
                    vkb, primitive.mesh, buffer.getCurrentBuffer(), buffer.count());
            }
        }
    }
}
