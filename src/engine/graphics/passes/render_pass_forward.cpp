#include "render_pass_forward.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassForward::RenderPassForward(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassForward"},
    buffer{buffer},
    resources{resources},
    pipelinePointCloud{vulkan, assetsManager},
    pipelineLines{vulkan, assetsManager},
    pipelinePolyShape{vulkan, assetsManager} {

    { // Depth
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::Depth, attachment);
    }

    { // Forward
        AttachmentInfo attachment{};
        attachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        addAttachment(RenderBufferPbr::Attachment::Forward, attachment);
    }

    addSubpass(
        {
            RenderBufferPbr::Attachment::Depth,
            RenderBufferPbr::Attachment::Forward,
        },
        {});

    { // Dependency for Forward
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }

    { // Dependency for Depth
        DependencyInfo dependency{};
        dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        addSubpassDependency(dependency);
    }

    addPipeline(pipelinePointCloud, 0);
    addPipeline(pipelineLines, 0);
    addPipeline(pipelinePolyShape, 0);
}

void RenderPassForward::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentPointCloud>(vkb, scene, jobs);
    collectForRender<ComponentPolyShape>(vkb, scene, jobs);
    collectForRender<ComponentLines>(vkb, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    currentPipeline = nullptr;
    for (auto& job : jobs) {
        job.fn();
    }
}

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                           ComponentTransform& transform, ComponentPointCloud& component) {
    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelinePointCloud) {
        pipelinePointCloud.bind(vkb);
        currentPipeline = &pipelinePointCloud;
    }

    pipelinePointCloud.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelinePointCloud.setTextureColor(component.getTexture()->getVulkanTexture());
    pipelinePointCloud.flushDescriptors(vkb);

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelinePointCloud.setModelMatrix(modelMatrix);
    pipelinePointCloud.flushConstants(vkb);

    pipelinePointCloud.renderMesh(vkb, mesh);
}

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                           ComponentTransform& transform, ComponentPolyShape& component) {
    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelinePolyShape) {
        pipelinePolyShape.bind(vkb);
        currentPipeline = &pipelinePolyShape;
    }

    pipelinePolyShape.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelinePolyShape.flushDescriptors(vkb);

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelinePolyShape.setModelMatrix(modelMatrix);
    pipelinePolyShape.flushConstants(vkb);

    pipelinePolyShape.renderMesh(vkb, mesh);
}

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                           ComponentTransform& transform, ComponentLines& component) {
    const auto& mesh = component.getMesh();
    if (mesh.count == 0) {
        return;
    }

    if (currentPipeline != &pipelineLines) {
        pipelineLines.bind(vkb);
        currentPipeline = &pipelineLines;
    }

    pipelineLines.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelineLines.flushDescriptors(vkb);

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelineLines.setModelMatrix(modelMatrix);
    pipelineLines.setColor(component.getColor());
    pipelineLines.flushConstants(vkb);

    pipelineLines.renderMesh(vkb, mesh);
}
