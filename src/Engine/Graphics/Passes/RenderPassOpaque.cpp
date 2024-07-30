#include "RenderPassOpaque.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Controllers/ControllerModelSkinned.hpp"
#include "../../Scene/Controllers/ControllerStaticModel.hpp"
#include "../../Scene/Scene.hpp"

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
                                   RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassOpaque"},
    buffer{buffer},
    resources{resources},
    pipelineGrid{vulkan},
    pipelineModel{vulkan},
    pipelineModelSkinned{vulkan},
    pipelineModelInstanced{vulkan},
    pipelinePlanet{vulkan} {

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

    { // Position
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.clearColor.color = {0.0f, 0.0f, 0.0f, 0.0f};
        addAttachment(RenderBufferPbr::Attachment::Position, attachment);
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
            RenderBufferPbr::Attachment::Position,
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
    addPipeline(pipelinePlanet, 0);

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
    renderPlanets(vkb, scene);
    renderModelsSkinned(vkb, scene);
    renderModelsInstanced(vkb, scene);
}

void RenderPassOpaque::renderGrids(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemGrids = scene.getView<ComponentTransform, ComponentGrid>(entt::exclude<TagDisabled>);
    auto& camera = *scene.getPrimaryCamera();

    pipelineGrid.bind(vkb);
    pipelineGrid.setDesriptorSet(vkb, 0, camera.getDescriptorSet());
    pipelineGrid.setDesriptorSet(vkb, 1, resources.getBlockMaterialsDescriptorSet());

    for (auto&& [entity, transform, grid] : systemGrids.each()) {
        const auto& mesh = grid.getMesh();
        if (!mesh) {
            continue;
        }

        const auto modelMatrix = transform.getAbsoluteInterpolatedTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineGrid.setModelMatrix(modelMatrix);
        pipelineGrid.setNormalMatrix(normalMatrix);
        pipelineGrid.setEntityColor(entityColor(transform));
        pipelineGrid.flushConstants(vkb);

        pipelineGrid.renderMesh(vkb, mesh);
    }
}

void RenderPassOpaque::renderModels(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemModels = scene.getView<ComponentTransform, ComponentModel>(entt::exclude<TagDisabled>);
    auto& camera = *scene.getPrimaryCamera();

    pipelineModel.bind(vkb);
    pipelineModel.setDesriptorSet(vkb, 0, camera.getDescriptorSet());

    for (auto&& [entity, transform, model] : systemModels.each()) {
        if (transform.isStatic()) {
            continue;
        }

        const auto modelMatrix = transform.getAbsoluteInterpolatedTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineModel.setModelMatrix(modelMatrix);
        pipelineModel.setNormalMatrix(normalMatrix);
        pipelineModel.setEntityColor(entityColor(transform));
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

                pipelineModel.setDesriptorSet(vkb, 1, primitive.material->descriptorSet);
                pipelineModel.renderMesh(vkb, primitive.mesh);
            }
        }
    }
}

void RenderPassOpaque::renderPlanets(VulkanCommandBuffer& vkb, Scene& scene) {
    auto systemPlanets = scene.getView<ComponentTransform, ComponentPlanet>(entt::exclude<TagDisabled>);
    auto& camera = *scene.getPrimaryCamera();

    pipelinePlanet.bind(vkb);

    for (auto&& [entity, transform, planet] : systemPlanets.each()) {
        if (planet.isBackground()) {
            continue;
        }

        const auto modelMatrix = transform.getAbsoluteInterpolatedTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));
        const auto& planetTextures = planet.getPlanetType()->getLowResTextures();

        pipelinePlanet.setModelMatrix(modelMatrix);
        pipelinePlanet.setNormalMatrix(normalMatrix);
        pipelinePlanet.flushConstants(vkb);

        pipelinePlanet.setUniformCamera(camera.getUbo().getCurrentBuffer());
        pipelinePlanet.setUniformAtmosphere(planet.getPlanetType()->getUbo());

        pipelinePlanet.setTextureBaseColor(planetTextures.getColor());
        pipelinePlanet.setTextureNormal(planetTextures.getNormal());
        pipelinePlanet.setTextureMetallicRoughness(planetTextures.getMetallicRoughness());
        pipelinePlanet.flushDescriptors(vkb);

        pipelinePlanet.renderMesh(vkb, resources.getMeshPlanet());
    }
}

void RenderPassOpaque::renderModelsSkinned(VulkanCommandBuffer& vkb, Scene& scene) {
    auto viewModelsSkinned = scene.getView<ComponentTransform, ComponentModelSkinned>(entt::exclude<TagDisabled>);
    auto& controllerModelsSkinned = scene.getController<ControllerModelSkinned>();
    auto& camera = *scene.getPrimaryCamera();

    pipelineModelSkinned.bind(vkb);
    pipelineModelSkinned.setDesriptorSet(vkb, 0, camera.getDescriptorSet());

    for (auto&& [entity, transform, component] : viewModelsSkinned.each()) {
        const auto& modelMatrix = transform.getAbsoluteInterpolatedTransform();
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3x3(modelMatrix)));

        pipelineModelSkinned.setModelMatrix(modelMatrix);
        pipelineModelSkinned.setNormalMatrix(normalMatrix);
        pipelineModelSkinned.setEntityColor(entityColor(transform));
        pipelineModelSkinned.flushConstants(vkb);

        for (const auto& node : component.getModel()->getNodes()) {
            // Skip non-animated models
            if (!node.skin) {
                continue;
            }

            for (auto& primitive : node.primitives) {
                if (!primitive.material) {
                    EXCEPTION("Primitive has no material");
                }

                validateMaterial(*primitive.material);

                std::array<uint32_t, 1> offsets{
                    static_cast<uint32_t>(component.getUboOffset()),
                };

                pipelineModelSkinned.setDesriptorSet(vkb, 1, primitive.material->descriptorSet);
                pipelineModelSkinned.setDesriptorSet(vkb, 2, controllerModelsSkinned.getDescriptorSet(), offsets);

                pipelineModelSkinned.renderMesh(vkb, primitive.mesh);
            }
        }
    }
}

void RenderPassOpaque::renderModelsInstanced(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& controllerStaticModel = scene.getController<ControllerStaticModel>();
    auto& camera = *scene.getPrimaryCamera();

    pipelineModelInstanced.bind(vkb);
    pipelineModelInstanced.setDesriptorSet(vkb, 0, camera.getDescriptorSet());

    for (auto&& [model, instances] : controllerStaticModel.getBuffers()) {
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

                pipelineModelInstanced.setDesriptorSet(vkb, 1, primitive.material->descriptorSet);
                pipelineModelInstanced.renderMeshInstanced(
                    vkb, primitive.mesh, instances.getCurrentBuffer(), instances.count());
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
