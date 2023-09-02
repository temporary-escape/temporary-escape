#include "render_pass_forward.hpp"
#include "../../assets/assets_manager.hpp"
#include "../../scene/controllers/controller_dynamics_world.hpp"
#include "../../scene/controllers/controller_turret.hpp"
#include "../../scene/scene.hpp"

using namespace Engine;

RenderPassForward::RenderPassForward(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     RenderResources& resources, AssetsManager& assetsManager) :
    RenderPass{vulkan, buffer, "RenderPassForward"},
    vulkan{vulkan},
    buffer{buffer},
    resources{resources},
    pipelinePointCloud{vulkan, assetsManager},
    pipelineLines{vulkan, assetsManager},
    pipelinePolyShape{vulkan, assetsManager},
    pipelineBullets{vulkan, assetsManager},
    pipelineBulletsTrail{vulkan, assetsManager},
    pipelineParticles{vulkan, assetsManager},
    pipelineDebug{vulkan, assetsManager} {

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
    addPipeline(pipelineBullets, 0);
    addPipeline(pipelineBulletsTrail, 0);
    addPipeline(pipelineParticles, 0);
    addPipeline(pipelineDebug, 0);
}

void RenderPassForward::render(VulkanCommandBuffer& vkb, Scene& scene) {
    vkb.setViewport({0, 0}, getViewport());
    vkb.setScissor({0, 0}, getViewport());

    std::vector<ForwardRenderJob> jobs;
    collectForRender<ComponentPointCloud>(vkb, scene, jobs);
    collectForRender<ComponentPolyShape>(vkb, scene, jobs);
    collectForRender<ComponentLines>(vkb, scene, jobs);
    collectForRender<ComponentGrid>(vkb, scene, jobs);

    std::sort(jobs.begin(), jobs.end(), [](auto& a, auto& b) { return a.order > b.order; });

    currentPipeline = nullptr;
    for (auto& job : jobs) {
        job.fn();
    }

    auto& camera = *scene.getPrimaryCamera();
    renderSceneBullets(vkb, camera, scene);
    renderSceneBulletsTrail(vkb, camera, scene);
    renderSceneDebug(vkb, camera, scene);
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

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, const ComponentCamera& camera,
                                           ComponentTransform& transform, ComponentGrid& component) {
    const auto& particles = component.getParticles();
    if (particles.empty()) {
        return;
    }

    const auto reg = component.getRegistry();
    if (!reg) {
        return;
    }

    const auto shipControl = reg->try_get<ComponentShipControl>(component.getHandle());
    if (!shipControl) {
        return;
    }

    auto strength = map(shipControl->getSpeed(), 0.0f, shipControl->getSpeedMax(), 0.0f, 1.0f);
    auto alpha = map(shipControl->getSpeed(), 0.0f, shipControl->getSpeedMax() / 5.0f, 0.0f, 1.0f);
    strength = glm::clamp(strength, 0.0f, 2.0f);
    alpha = glm::clamp(alpha, 0.0f, 1.0f);

    if (currentPipeline != &pipelineParticles) {
        pipelineParticles.bind(vkb);
        currentPipeline = &pipelineParticles;
    }

    for (const auto& [particlesType, matrices] : particles) {

        pipelineParticles.setUniformCamera(camera.getUbo().getCurrentBuffer());
        pipelineParticles.setUniformParticlesType(particlesType->getUbo());
        pipelineParticles.setTextureColor(particlesType->getTexture()->getVulkanTexture());
        pipelineParticles.flushDescriptors(vkb);

        for (const auto& matrix : matrices) {
            const auto modelMatrix = transform.getAbsoluteTransform() * matrix;
            pipelineParticles.setModelMatrix(modelMatrix);
            pipelineParticles.setTimeDelta(vulkan.getRenderTime());
            pipelineParticles.setOverrideStrength(strength);
            pipelineParticles.setOverrideAlpha(alpha);
            pipelineParticles.flushConstants(vkb);

            vkb.draw(4, particlesType->getCount(), 0, 0);
        }
    }
}

void RenderPassForward::renderSceneBullets(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Scene& scene) {
    const auto& controller = scene.getController<ControllerTurret>();
    const auto& vboInstances = controller.getVboBullets();
    const auto& meshBullet = resources.getMeshBullet();
    const auto count = controller.getBulletsCount();

    if (count == 0 || !vboInstances) {
        return;
    }

    pipelineBullets.bind(vkb);

    pipelineBullets.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelineBullets.flushDescriptors(vkb);

    std::array<VulkanVertexBufferBindRef, 2> vboBindings{};
    vboBindings[0] = {&meshBullet.vbo, 0};
    vboBindings[1] = {&vboInstances.getCurrentBuffer(), 0};
    vkb.bindBuffers(vboBindings);

    vkb.draw(meshBullet.count, count, 0, 0);
}

void RenderPassForward::renderSceneBulletsTrail(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Scene& scene) {
    const auto& controller = scene.getController<ControllerTurret>();
    const auto& vboInstances = controller.getVboBullets();
    const auto& meshBullet = resources.getMeshBullet();
    const auto count = controller.getBulletsCount();

    if (count == 0 || !vboInstances) {
        return;
    }

    pipelineBulletsTrail.bind(vkb);

    pipelineBulletsTrail.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelineBulletsTrail.flushDescriptors(vkb);

    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
    vboBindings[0] = {&vboInstances.getCurrentBuffer(), 0};
    vkb.bindBuffers(vboBindings);

    vkb.draw(2, count, 0, 0);
}

void RenderPassForward::renderSceneDebug(VulkanCommandBuffer& vkb, const ComponentCamera& camera, Scene& scene) {
    const auto& controller = scene.getController<ControllerDynamicsWorld>();
    const auto& vbo = controller.getDebugDrawVbo();
    const auto count = controller.getDebugDrawCount();

    if (count == 0 || !vbo) {
        return;
    }

    pipelineDebug.bind(vkb);

    Matrix4 modelMatrix{1.0f};

    pipelineDebug.setModelMatrix(modelMatrix);
    pipelineDebug.flushConstants(vkb);
    pipelineDebug.setUniformCamera(camera.getUbo().getCurrentBuffer());
    pipelineDebug.flushDescriptors(vkb);

    std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
    vboBindings[0] = {&vbo.getCurrentBuffer(), 0};
    vkb.bindBuffers(vboBindings);

    vkb.draw(count * 2, 1, 0, 0);
}
