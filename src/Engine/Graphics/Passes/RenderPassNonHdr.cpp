#include "RenderPassNonHdr.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Controllers/ControllerIcon.hpp"
#include "../../Scene/Scene.hpp"
#include "../Theme.hpp"

using namespace Engine;

RenderPassNonHDR::RenderPassNonHDR(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                   RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassNonHDR"},
    options{options},
    buffer{buffer},
    resources{resources},
    pipelineOutline{vulkan},
    pipelineIcons{vulkan},
    pipelineWorldText{vulkan} {

    { // Forward
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(options.fxaa ? RenderBufferPbr::Attachment::Forward : RenderBufferPbr::Attachment::FXAA,
                      attachment);
    }

    addSubpass(
        {
            options.fxaa ? RenderBufferPbr::Attachment::Forward : RenderBufferPbr::Attachment::FXAA,
        },
        {});

    { // Dependency for Forward/FXAA
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }

    addPipeline(pipelineOutline, 0);
    addPipeline(pipelineIcons, 0);
    addPipeline(pipelineWorldText, 0);
}

void RenderPassNonHDR::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPassNonHDR::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    renderOutline(vkb, scene);
    renderWorldText(vkb, scene);
    renderIcons(vkb, scene);
}

void RenderPassNonHDR::renderOutline(VulkanCommandBuffer& vkb, Scene& scene) {
    const auto selected = scene.getSelectedEntity();
    if (!selected) {
        return;
    }

    pipelineOutline.bind(vkb);

    const auto& texEntity = buffer.getAttachmentTexture(RenderBufferPbr::Attachment::Entity);
    const auto selectedColor = entityColor(selected->getHandle());
    const auto finalColor = Theme::primary * alpha(0.2);

    pipelineOutline.setColorFinal(finalColor);
    pipelineOutline.setColorSelected(selectedColor);
    pipelineOutline.setThickness(0.6f);
    pipelineOutline.flushConstants(vkb);
    pipelineOutline.setTextureEntity(texEntity);
    pipelineOutline.flushDescriptors(vkb);

    pipelineOutline.renderMesh(vkb, resources.getMeshFullScreenQuad());
}

void RenderPassNonHDR::renderIcons(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();
    auto& controllerIcons = scene.getController<ControllerIcon>();

    std::array<const ControllerIcon::Buffers*, 2> buffers{};
    buffers[0] = &controllerIcons.getStaticBuffers();
    buffers[1] = &controllerIcons.getDynamicBuffers();

    if (buffers[0]->empty() && buffers[1]->empty()) {
        return;
    }

    pipelineIcons.bind(vkb);

    const auto modelMatrix = Matrix4{1.0f};
    const float scale = camera.isOrthographic() ? 110.0f : 1.0f; // TODO: WTF?

    pipelineIcons.setModelMatrix(modelMatrix);
    pipelineIcons.setScale(scale);
    pipelineIcons.flushConstants(vkb);

    for (const auto* b : buffers) {
        for (const auto& pair : *b) {
            if (pair.second.count() == 0) {
                continue;
            }

            pipelineIcons.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineIcons.setTextureColor(*pair.first);
            pipelineIcons.flushDescriptors(vkb);

            std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
            vboBindings[0] = {&pair.second.getCurrentBuffer(), 0};
            vkb.bindBuffers(vboBindings);

            vkb.draw(4, pair.second.count(), 0, 0);
        }
    }
}

void RenderPassNonHDR::renderWorldText(VulkanCommandBuffer& vkb, Scene& scene) {
    auto& camera = *scene.getPrimaryCamera();

    pipelineWorldText.bind(vkb);

    for (auto&& [entity, transform, worldText] : scene.getView<ComponentTransform, ComponentWorldText>().each()) {
        const auto& mesh = worldText.getMesh();

        if (mesh.count == 0) {
            return;
        }

        const auto modelMatrix = transform.getAbsoluteTransform();

        pipelineWorldText.setModelMatrix(modelMatrix);
        pipelineWorldText.setColor(worldText.getColor());
        pipelineWorldText.flushConstants(vkb);

        pipelineWorldText.setUniformCamera(camera.getUbo().getCurrentBuffer());
        pipelineWorldText.setTextureColor(worldText.getFontFace().getTexture());
        pipelineWorldText.flushDescriptors(vkb);

        pipelineWorldText.renderMesh(vkb, mesh);
    }
}
