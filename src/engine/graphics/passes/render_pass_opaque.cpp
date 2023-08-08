#include "render_pass_opaque.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/controllers/controller_static_model.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

static void validateMaterial(const Material& material) {
    if (!material.ubo) {
        EXCEPTION("Primitive has no material uniform buffer allocated");
    }

    if (!material.baseColorTexture || !material.baseColorTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no base color texture");
    }

    if (!material.emissiveTexture || !material.emissiveTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no emissive texture");
    }

    if (!material.normalTexture || !material.normalTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no normal texture");
    }

    if (!material.ambientOcclusionTexture || !material.ambientOcclusionTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no ambient occlusion texture");
    }

    if (!material.metallicRoughnessTexture || !material.metallicRoughnessTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no metallic roughness texture");
    }

    if (!material.maskTexture || !material.maskTexture->getVulkanTexture()) {
        EXCEPTION("Primitive has no mask texture");
    }
}

RenderPassOpaque::RenderPassOpaque(VulkanRenderer& vulkan, RenderBufferPbr& buffer, RenderResources& resources,
                                   AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassOpaque"},
    buffer{buffer},
    resources{resources},
    pipelineGrid{vulkan, assetsManager},
    pipelineModel{vulkan, assetsManager},
    pipelineModelInstanced{vulkan, assetsManager} {

    { // Depth
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.clearColor.depthStencil = {1.0f, 0x00};
        addAttachment(RenderBufferPbr::Attachment::Depth, attachment);
    }

    { // AlbedoAmbient
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 0.0f};
        addAttachment(RenderBufferPbr::Attachment::AlbedoAmbient, attachment);
    }

    { // Emissive Roughness
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 0.0f};
        addAttachment(RenderBufferPbr::Attachment::EmissiveRoughness, attachment);
    }

    { // Normal Metallic
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 0.0f};
        addAttachment(RenderBufferPbr::Attachment::NormalMetallic, attachment);
    }

    { // Entity
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {1.0f, 1.0f, 1.0f, 1.0f};
        addAttachment(RenderBufferPbr::Attachment::Entity, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::Depth,
            RenderBufferPbr::Attachment::AlbedoAmbient,
            RenderBufferPbr::Attachment::EmissiveRoughness,
            RenderBufferPbr::Attachment::NormalMetallic,
            RenderBufferPbr::Attachment::Entity,
        },
        {});

    { // Dependency for Depth
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }

    addPipeline(pipelineGrid, 0);
    addPipeline(pipelineModel, 0);
    addPipeline(pipelineModelInstanced, 0);

    palette = assetsManager.getTextures().find("palette");
}

void RenderPassOpaque::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassOpaque::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderGrids(vkb, scene);
    renderModels(vkb, scene);
    renderModelsInstanced(vkb, scene);
}

void RenderPassOpaque::renderGrids(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>();
    auto& camera = *scene.getPrimaryCamera();

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

            validateMaterial(*primitive.material);

            pipelineGrid.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineGrid.setUniformMaterial(primitive.material->ubo);

            pipelineGrid.setTextureBaseColor(primitive.material->baseColorTexture->getVulkanTexture());
            pipelineGrid.setTextureEmissive(primitive.material->emissiveTexture->getVulkanTexture());
            pipelineGrid.setTextureNormal(primitive.material->normalTexture->getVulkanTexture());
            pipelineGrid.setTextureAmbientOcclusion(primitive.material->ambientOcclusionTexture->getVulkanTexture());
            pipelineGrid.setTextureMetallicRoughness(primitive.material->metallicRoughnessTexture->getVulkanTexture());
            pipelineGrid.setTextureMask(primitive.material->maskTexture->getVulkanTexture());
            pipelineGrid.setTexturePalette(palette->getVulkanTexture());
            pipelineGrid.flushDescriptors(vkb);

            pipelineGrid.renderMesh(vkb, primitive.mesh);
        }
    }
}

void RenderPassOpaque::renderModels(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemModels = scene.getView<ComponentTransform, ComponentModel>();
    auto& camera = *scene.getPrimaryCamera();

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

        for (auto& primitive : model.getModel()->getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            validateMaterial(*primitive.material);

            pipelineModel.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineModel.setUniformMaterial(primitive.material->ubo);

            pipelineModel.setTextureBaseColor(primitive.material->baseColorTexture->getVulkanTexture());
            pipelineModel.setTextureEmissive(primitive.material->emissiveTexture->getVulkanTexture());
            pipelineModel.setTextureNormal(primitive.material->normalTexture->getVulkanTexture());
            pipelineModel.setTextureAmbientOcclusion(primitive.material->ambientOcclusionTexture->getVulkanTexture());
            pipelineModel.setTextureMetallicRoughness(primitive.material->metallicRoughnessTexture->getVulkanTexture());
            pipelineModel.flushDescriptors(vkb);

            pipelineModel.renderMesh(vkb, primitive.mesh);
        }
    }
}

void RenderPassOpaque::renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerStaticModel = scene.getController<ControllerStaticModel>();
    auto& camera = *scene.getPrimaryCamera();

    pipelineModelInstanced.bind(vkb);

    for (auto&& [model, buffer] : controllerStaticModel.getBuffers()) {
        for (auto& primitive : model->getPrimitives()) {
            if (!primitive.material) {
                EXCEPTION("Primitive has no material");
            }

            validateMaterial(*primitive.material);

            pipelineModelInstanced.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineModelInstanced.setUniformMaterial(primitive.material->ubo);

            pipelineModelInstanced.setTextureBaseColor(primitive.material->baseColorTexture->getVulkanTexture());
            pipelineModelInstanced.setTextureEmissive(primitive.material->emissiveTexture->getVulkanTexture());
            pipelineModelInstanced.setTextureNormal(primitive.material->normalTexture->getVulkanTexture());
            pipelineModelInstanced.setTextureAmbientOcclusion(
                primitive.material->ambientOcclusionTexture->getVulkanTexture());
            pipelineModelInstanced.setTextureMetallicRoughness(
                primitive.material->metallicRoughnessTexture->getVulkanTexture());
            pipelineModelInstanced.flushDescriptors(vkb);

            pipelineModelInstanced.renderMeshInstanced(vkb, primitive.mesh, buffer.getCurrentBuffer(), buffer.count());
        }
    }
}
