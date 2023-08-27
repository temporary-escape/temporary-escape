#include "render_pass_opaque.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/controllers/controller_model_skinned.hpp"
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

RenderPassOpaque::RenderPassOpaque(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                   RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassOpaque"},
    buffer{buffer},
    resources{resources},
    pipelineGrid{vulkan, assetsManager},
    pipelineModel{vulkan, assetsManager},
    pipelineModelSkinned{vulkan, assetsManager},
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
    addPipeline(pipelineModelSkinned, 0);
    addPipeline(pipelineModelInstanced, 0);

    palette = assetsManager.getTextures().find("palette");

    VulkanBuffer::CreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = 4; // 1 pixel RGBA
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO;
    bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    entityColorBuffer = vulkan.createDoubleBuffer(bufferInfo);
}

void RenderPassOpaque::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassOpaque::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderGrids(vkb, scene);
    renderModels(vkb, scene);
    renderModelsSkinned(vkb, scene);
    renderModelsInstanced(vkb, scene);
}

void RenderPassOpaque::renderGrids(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>(entt::exclude<TagDisabled>);
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
            if (!primitive.mesh) {
                continue;
            }

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
    auto systemModels = scene.getView<ComponentTransform, ComponentModel>(entt::exclude<TagDisabled>);
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

        for (const auto& node : model.getModel()->getNodes()) {
            // Skip animated models
            if (node.skin) {
                continue;
            }

            for (auto& primitive : node.primitives) {
                if (!primitive.material) {
                    EXCEPTION("Primitive has no material");
                }

                validateMaterial(*primitive.material);

                pipelineModel.setUniformCamera(camera.getUbo().getCurrentBuffer());
                pipelineModel.setUniformMaterial(primitive.material->ubo);

                pipelineModel.setTextureBaseColor(primitive.material->baseColorTexture->getVulkanTexture());
                pipelineModel.setTextureEmissive(primitive.material->emissiveTexture->getVulkanTexture());
                pipelineModel.setTextureNormal(primitive.material->normalTexture->getVulkanTexture());
                pipelineModel.setTextureAmbientOcclusion(
                    primitive.material->ambientOcclusionTexture->getVulkanTexture());
                pipelineModel.setTextureMetallicRoughness(
                    primitive.material->metallicRoughnessTexture->getVulkanTexture());
                pipelineModel.flushDescriptors(vkb);

                pipelineModel.renderMesh(vkb, primitive.mesh);
            }
        }
    }
}

void RenderPassOpaque::renderModelsSkinned(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerModelsSkinned = scene.getController<ControllerModelSkinned>();
    auto& camera = *scene.getPrimaryCamera();

    pipelineModelSkinned.bind(vkb);

    for (const auto& item : controllerModelsSkinned.getItems()) {
        const auto& modelMatrix = item.transform;
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineModelSkinned.setModelMatrix(modelMatrix);
        pipelineModelSkinned.setNormalMatrix(normalMatrix);
        pipelineModelSkinned.setEntityColor(entityColor(item.entity));
        pipelineModelSkinned.flushConstants(vkb);

        for (const auto& node : item.model->getNodes()) {
            // Skip non-animated models
            if (!node.skin) {
                continue;
            }

            for (auto& primitive : node.primitives) {
                if (!primitive.material) {
                    EXCEPTION("Primitive has no material");
                }

                validateMaterial(*primitive.material);

                pipelineModelSkinned.setUniformCamera(camera.getUbo().getCurrentBuffer());
                pipelineModelSkinned.setUniformMaterial(primitive.material->ubo);
                pipelineModelSkinned.setUniformArmature(controllerModelsSkinned.getUbo().getCurrentBuffer(),
                                                        item.offset);

                pipelineModelSkinned.setTextureBaseColor(primitive.material->baseColorTexture->getVulkanTexture());
                pipelineModelSkinned.setTextureEmissive(primitive.material->emissiveTexture->getVulkanTexture());
                pipelineModelSkinned.setTextureNormal(primitive.material->normalTexture->getVulkanTexture());
                pipelineModelSkinned.setTextureAmbientOcclusion(
                    primitive.material->ambientOcclusionTexture->getVulkanTexture());
                pipelineModelSkinned.setTextureMetallicRoughness(
                    primitive.material->metallicRoughnessTexture->getVulkanTexture());
                pipelineModelSkinned.flushDescriptors(vkb);

                pipelineModelSkinned.renderMesh(vkb, primitive.mesh);
            }
        }
    }
}

void RenderPassOpaque::renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerStaticModel = scene.getController<ControllerStaticModel>();
    auto& camera = *scene.getPrimaryCamera();

    pipelineModelInstanced.bind(vkb);

    for (auto&& [model, buffer] : controllerStaticModel.getBuffers()) {
        for (const auto& node : model->getNodes()) {
            // Skip animated models
            if (node.skin) {
                continue;
            }

            for (auto& primitive : node.primitives) {
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

                pipelineModelInstanced.renderMeshInstanced(
                    vkb, primitive.mesh, buffer.getCurrentBuffer(), buffer.count());
            }
        }
    }
}

void RenderPassOpaque::afterRender(VulkanCommandBuffer& vkb) {
    const auto& texture = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Entity);
    const auto viewport = texture.getSize2D();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);

    if (mousePos.x >= 0 && mousePos.x < viewport.x && mousePos.y >= 0 && mousePos.y < viewport.y) {
        std::array<VkBufferImageCopy, 1> regions{};
        regions[0].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[0].imageSubresource.layerCount = 1;
        regions[0].imageExtent.width = 1;
        regions[0].imageExtent.height = 1;
        regions[0].imageExtent.depth = 1;
        regions[0].imageOffset.x = mousePos.x;
        regions[0].imageOffset.y = mousePos.y;
        regions[0].imageOffset.z = 0;

        vkb.copyImageToBuffer(
            texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, entityColorBuffer.getCurrentBuffer(), regions);
    }

    barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture.getHandle();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
}

uint32_t RenderPassOpaque::getMousePosEntity() const {
    const auto* src = static_cast<const unsigned char*>(entityColorBuffer.getCurrentBuffer().getMappedPtr());
    uint32_t id{0};
    id |= static_cast<uint32_t>(src[0]);
    id |= static_cast<uint32_t>(src[1]) << 8;
    id |= static_cast<uint32_t>(src[2]) << 16;
    id |= static_cast<uint32_t>(src[3]) << 24;
    return id;
}
