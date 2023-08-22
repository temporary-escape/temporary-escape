#include "render_pass.hpp"
#include "../utils/exceptions.hpp"
#include "render_pipeline.hpp"

using namespace Engine;

RenderPass::RenderPass(VulkanRenderer& vulkan, RenderBuffer& renderBuffer, std::string name) :
    vulkan{vulkan}, renderBuffer{renderBuffer}, name{std::move(name)} {
}

void RenderPass::create() {
    for (const auto& [pipeline, _] : pipelines) {
        isCompute |= pipeline->isCompute();
    }

    if (!isCompute) {
        createRenderPass();
        createFbo();
    }

    for (const auto& [pipeline, subpass] : pipelines) {
        try {
            if (isCompute) {
                pipeline->create(renderPass, subpass, {});
            } else {
                const auto& subpassData = *std::next(subpassDescriptionData.begin(), subpass);
                pipeline->create(renderPass, subpass, subpassData.attachments);
            }
        } catch (...) {
            EXCEPTION_NESTED("Failed to create render pipeline: {}", pipeline->getName());
        }
    }
}

void RenderPass::begin(VulkanCommandBuffer& vkb) {
    beforeRender(vkb);

    for (auto& [pipeline, subpass] : pipelines) {
        pipeline->getDescriptionPool().reset();
    }

    if (!isCompute) {
        renderPassBeginInfo.framebuffer = &getFbo();
        renderPassBeginInfo.renderPass = &getRenderPass();
        renderPassBeginInfo.offset = {0, 0};
        renderPassBeginInfo.size = {viewport.width, viewport.height};

        vkb.beginRenderPass(renderPassBeginInfo);
    }
}

void RenderPass::end(VulkanCommandBuffer& vkb) {
    if (!isCompute) {
        vkb.endRenderPass();
    }

    afterRender(vkb);
}

void RenderPass::addPipeline(RenderPipeline& pipeline, const uint32_t subpass) {
    pipelines.emplace_back(&pipeline, subpass);
}

void RenderPass::addAttachment(uint32_t attachment, const AttachmentInfo& info) {
    const auto& view = renderBuffer.getAttachment(attachment);
    const auto& texture = renderBuffer.getAttachmentTexture(attachment);
    const auto extent = texture.getExtent();

    if (viewport.width == 0) {
        viewport.width = extent.width;
    } else {
        viewport.width = std::min(viewport.width, extent.width);
    }
    if (viewport.height == 0) {
        viewport.height = extent.height;
    } else {
        viewport.height = std::min(viewport.height, extent.height);
    }

    attachmentViews.push_back(view.getHandle());

    if (isDepthFormat(texture.getFormat())) {
        depthAttachment = attachment;
    }

    VkAttachmentDescription attachmentDescription{};
    attachmentDescription.format = texture.getFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = info.loadOp;
    attachmentDescription.storeOp = info.storeOp;
    attachmentDescription.stencilLoadOp = info.stencilLoadOp;
    attachmentDescription.stencilStoreOp = info.stencilStoreOp;
    attachmentDescription.initialLayout = info.initialLayout;
    attachmentDescription.finalLayout = info.finalLayout;
    attachmentDescriptions.push_back(attachmentDescription);

    attachmentIndexes.push_back(attachment);
    renderPassBeginInfo.clearValues.push_back(info.clearColor);
}

uint32_t RenderPass::getAttachmentIndex(const uint32_t attachment) const {
    for (size_t i = 0; i < attachmentIndexes.size(); i++) {
        if (attachment == attachmentIndexes.at(i)) {
            return i;
        }
    }
    EXCEPTION("No such attachment found: {}", static_cast<size_t>(attachment));
}

void RenderPass::addSubpass(const std::vector<uint32_t>& attachments, const std::vector<uint32_t>& inputs) {
    subpassDescriptionData.emplace_back();
    auto& data = subpassDescriptionData.back();

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (const auto& attachment : attachments) {
        const auto index = getAttachmentIndex(attachment);

        if (index >= attachmentViews.size()) {
            EXCEPTION("Subpass attachment index: {} out of range", index);
        }

        if (attachment == depthAttachment) {
            data.depthReference = {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            subpassDescription.pDepthStencilAttachment = &data.depthReference;
        } else {
            data.colorReferences.push_back({index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }

        data.attachments.push_back(index);
    }

    for (const auto& attachment : inputs) {
        const auto index = getAttachmentIndex(attachment);

        if (index >= attachments.size()) {
            EXCEPTION("Subpass input index: {} out of range", index);
        }

        data.inputsReferences.push_back({index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        data.inputs.push_back(index);
    }

    if (!data.colorReferences.empty()) {
        subpassDescription.colorAttachmentCount = static_cast<uint32_t>(data.colorReferences.size());
        subpassDescription.pColorAttachments = data.colorReferences.data();
    }

    if (!data.inputsReferences.empty()) {
        subpassDescription.inputAttachmentCount = static_cast<uint32_t>(data.inputsReferences.size());
        subpassDescription.pInputAttachments = data.inputsReferences.data();
    }

    if (!subpassDescriptions.empty()) {
        // Additional subpasses
        VkSubpassDependency dependency{};
        dependency.srcSubpass = subpassDescriptions.size() - 1;
        dependency.dstSubpass = subpassDescriptions.size();
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);
    }

    subpassDescriptions.push_back(subpassDescription);
}

void RenderPass::addSubpassDependency(const RenderPass::DependencyInfo& dependency) {
    dependencies.emplace_back();
    auto& dep = dependencies.back();
    dep.dependencyFlags = 0;
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = dependency.srcStageMask;
    dep.dstStageMask = dependency.dstStageMask;
    dep.srcAccessMask = dependency.srcAccessMask;
    dep.dstAccessMask = dependency.dstAccessMask;
}

void RenderPass::createRenderPass() {
    if (subpassDescriptions.empty()) {
        EXCEPTION("Can not create render pass with no subpass descriptions");
    }

    VulkanRenderPass::CreateInfo renderPassInfo = {};
    renderPassInfo.attachments = attachmentDescriptions;
    renderPassInfo.dependencies = dependencies;
    renderPassInfo.subPasses = subpassDescriptions;

    renderPass = vulkan.createRenderPass(renderPassInfo);
}

void RenderPass::createFbo() {
    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = viewport.width;
    framebufferInfo.height = viewport.height;
    framebufferInfo.layers = 1;

    fbo = vulkan.createFramebuffer(framebufferInfo);
}

void RenderPass::beforeRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}

void RenderPass::afterRender(VulkanCommandBuffer& vkb) {
    (void)vkb;
}
