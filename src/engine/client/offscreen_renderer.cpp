#include "offscreen_renderer.hpp"
#include "../scene/scene.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

class CustomRenderer : public Renderer {
public:
    explicit CustomRenderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan, Canvas& canvas,
                            Nuklear& nuklear, ShaderModules& shaderModules, VoxelShapeCache& voxelShapeCache,
                            Registry& registry, FontFamily& font, VulkanFramebuffer& fbo,
                            VulkanRenderPass& renderPass) :
        Renderer{config, viewport, vulkan, canvas, nuklear, shaderModules, voxelShapeCache, registry, font},
        fbo{fbo},
        renderPass{renderPass} {
    }

protected:
    VulkanFramebuffer& getParentFramebuffer() override {
        return fbo;
    }

    VulkanRenderPass& getParentRenderPass() override {
        return renderPass;
    }

    VulkanFenceOpt getParentInFlightFence() override {
        return std::nullopt;
    }

    VulkanSemaphoreOpt getParentImageAvailableSemaphore() override {
        return std::nullopt;
    }

    VulkanSemaphoreOpt getParentRenderFinishedSemaphore() override {
        return std::nullopt;
    }

private:
    VulkanFramebuffer& fbo;
    VulkanRenderPass& renderPass;
};

OffscreenRenderer::OffscreenRenderer(const Config& config, const Vector2i& viewport, VulkanRenderer& vulkan,
                                     Canvas& canvas, Nuklear& nuklear, ShaderModules& shaderModules,
                                     VoxelShapeCache& voxelShapeCache, Registry& registry, FontFamily& font) :
    viewport{viewport}, vulkan{vulkan}, skybox{vulkan, Color4{0.05f, 0.05f, 0.05f, 1.0f}} {

    createRenderPass(vulkan);
    createTexture(vulkan, viewport);
    createFramebuffer(vulkan, viewport);

    renderer = std::make_shared<CustomRenderer>(config, viewport, vulkan, canvas, nuklear, shaderModules,
                                                voxelShapeCache, registry, font, fbo, renderPass);
}

void OffscreenRenderer::createRenderPass(VulkanRenderer& vulkan) {
    std::vector<VkAttachmentDescription> attachments{1};
    attachments[0].format = VK_FORMAT_R8G8B8A8_SRGB;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VulkanRenderPass::CreateInfo renderPassInfo{};
    renderPassInfo.attachments = attachments;

    std::array<VkAttachmentReference, 1> colorAttachmentRefs{};

    colorAttachmentRefs[0].attachment = 0;
    colorAttachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Use subpass dependencies for attachment layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();

    renderPassInfo.subPasses = {subpass};
    renderPassInfo.dependencies = {dependency};

    renderPass = vulkan.createRenderPass(renderPassInfo);
}

void OffscreenRenderer::createFramebuffer(VulkanRenderer& vulkan, const Vector2i& viewport) {
    std::array<VkImageView, 1> attachmentViews = {
        fboColor.getImageView(),
    };

    VulkanFramebuffer::CreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass.getHandle();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = viewport.x;
    framebufferInfo.height = viewport.y;
    framebufferInfo.layers = 1;

    fbo = vulkan.createFramebuffer(framebufferInfo);
}

void OffscreenRenderer::createTexture(VulkanRenderer& vulkan, const Vector2i& viewport) {
    VulkanTexture::CreateInfo textureInfo{};
    textureInfo.image.format = VK_FORMAT_R8G8B8A8_SRGB;
    textureInfo.image.imageType = VK_IMAGE_TYPE_2D;
    textureInfo.image.extent = {static_cast<uint32_t>(viewport.x), static_cast<uint32_t>(viewport.y), 1};
    textureInfo.image.mipLevels = 1;
    textureInfo.image.arrayLayers = 1;
    textureInfo.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureInfo.image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureInfo.image.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureInfo.image.samples = VK_SAMPLE_COUNT_1_BIT;
    textureInfo.image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    textureInfo.view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    textureInfo.view.format = VK_FORMAT_R8G8B8A8_SRGB;
    textureInfo.view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    textureInfo.view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    textureInfo.view.subresourceRange.baseMipLevel = 0;
    textureInfo.view.subresourceRange.levelCount = 1;
    textureInfo.view.subresourceRange.baseArrayLayer = 0;
    textureInfo.view.subresourceRange.layerCount = 1;

    textureInfo.sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    textureInfo.sampler.magFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.minFilter = VK_FILTER_LINEAR;
    textureInfo.sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    textureInfo.sampler.anisotropyEnable = VK_FALSE;
    textureInfo.sampler.maxAnisotropy = 1.0f;
    textureInfo.sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    textureInfo.sampler.unnormalizedCoordinates = VK_FALSE;
    textureInfo.sampler.compareEnable = VK_FALSE;
    textureInfo.sampler.compareOp = VK_COMPARE_OP_ALWAYS;
    textureInfo.sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    textureInfo.sampler.minLod = 0.0f;
    textureInfo.sampler.maxLod = 0.0f;

    fboColor = vulkan.createTexture(textureInfo);
}

void OffscreenRenderer::render(const BlockPtr& block, VoxelShape::Type shape) {
    logger.info("Rendering thumbnail for block: {}", block ? block->getName() : "nullptr");

    Scene scene{};

    auto sun = scene.createEntity();
    sun->addComponent<ComponentDirectionalLight>(Color4{1.5f, 1.5f, 1.5f, 1.0f});
    sun->addComponent<ComponentTransform>().translate(Vector3{3.0f, 3.0f, 1.0f});

    auto entityCamera = scene.createEntity();
    auto& cameraTransform = entityCamera->addComponent<ComponentTransform>();
    auto& cameraCamera = entityCamera->addComponent<ComponentCamera>(cameraTransform);
    entityCamera->addComponent<ComponentUserInput>(cameraCamera);
    cameraCamera.setProjection(19.0f);
    cameraCamera.lookAt({3.0f, 3.0f, 3.0f}, {0.0f, 0.0f, 0.0f});
    scene.setPrimaryCamera(entityCamera);

    if (block) {
        auto entityBlock = scene.createEntity();
        auto& entityTransform = entityBlock->addComponent<ComponentTransform>();
        auto& grid = entityBlock->addComponent<ComponentGrid>();
        grid.setDirty(true);
        grid.insert(Vector3i{0, 0, 0}, block, 0, 0, shape);
    }

    Renderer::Options options{};
    NuklearWindowNull gui{};

    scene.update(0.1f);

    renderer->render(viewport, scene, skybox, options, gui);

    vulkan.waitQueueIdle();
}
