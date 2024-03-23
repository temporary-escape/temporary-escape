#include "RenderPassForward.hpp"
#include "../../Assets/AssetsManager.hpp"
#include "../../Scene/Controllers/ControllerBullets.hpp"
#include "../../Scene/Controllers/ControllerIcon.hpp"
#include "../../Scene/Controllers/ControllerTurret.hpp"
#include "../../Scene/Scene.hpp"

using namespace Engine;

RenderPassForward::RenderPassForward(const RenderOptions& options, VulkanRenderer& vulkan, RenderBufferPbr& buffer,
                                     RenderResources& resources) :
    RenderPass{vulkan, buffer, "RenderPassForward"},
    vulkan{vulkan},
    buffer{buffer},
    resources{resources},
    pipelinePointCloud{vulkan},
    pipelineLines{vulkan},
    pipelinePolyShape{vulkan},
    pipelineBullets{vulkan},
    pipelineBulletsTrail{vulkan},
    pipelineParticles{vulkan},
    pipelineDebug{vulkan},
    pipelineSpaceDust{vulkan},
    pipelineTacticalOverlayLines{vulkan},
    pipelineTacticalOverlaySpots{vulkan} {

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
    addPipeline(pipelineSpaceDust, 0);
    addPipeline(pipelineTacticalOverlayLines, 0);
    addPipeline(pipelineTacticalOverlaySpots, 0);
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
    renderSceneBullets(vkb, scene, camera);
    renderSceneBulletsTrail(vkb, scene, camera);
    renderSceneDebug(vkb, scene, camera);
    renderSceneSpaceDust(vkb, scene, camera);
    renderSceneTacticalOverlay(vkb, scene, camera);
    renderSceneShipControls(vkb, scene, camera);
}

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
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
    pipelinePointCloud.setTextureColor(component.getTexture());
    pipelinePointCloud.flushDescriptors(vkb);

    const auto modelMatrix = transform.getAbsoluteTransform();
    pipelinePointCloud.setModelMatrix(modelMatrix);
    pipelinePointCloud.flushConstants(vkb);

    pipelinePointCloud.renderMesh(vkb, mesh);
}

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
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

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
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

void RenderPassForward::renderSceneForward(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera,
                                           ComponentTransform& transform, ComponentGrid& component) {
    const auto& particles = component.getParticles();
    if (particles.empty()) {
        return;
    }

    const auto* shipControl = scene.tryGetComponent<ComponentShipControl>(component.getEntity());
    if (!shipControl) {
        return;
    }

    auto strength = map(shipControl->getForwardVelocity(), 0.0f, shipControl->getForwardVelocityMax(), 0.0f, 3.0f);
    auto alpha = map(shipControl->getForwardVelocity(), 0.0f, shipControl->getForwardVelocityMax() / 5.0f, 0.0f, 1.0f);
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
            const auto modelMatrix = transform.getAbsoluteInterpolatedTransform() * matrix;
            pipelineParticles.setModelMatrix(modelMatrix);
            pipelineParticles.setTimeDelta(vulkan.getRenderTime());
            pipelineParticles.setOverrideStrength(strength);
            pipelineParticles.setOverrideAlpha(alpha);
            pipelineParticles.flushConstants(vkb);

            vkb.draw(4, particlesType->getCount(), 0, 0);
        }
    }
}

void RenderPassForward::renderSceneBullets(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera) {
    const auto& controller = scene.getController<ControllerBullets>();
    const auto& vboInstances = controller.getVbo();
    const auto& meshBullet = resources.getMeshBullet();
    const auto count = controller.getCount();

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

void RenderPassForward::renderSceneBulletsTrail(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera) {
    const auto& controller = scene.getController<ControllerBullets>();
    const auto& vboInstances = controller.getVbo();
    const auto& meshBullet = resources.getMeshBullet();
    const auto count = controller.getCount();

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

void RenderPassForward::renderSceneDebug(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera) {
    const auto& dynamicsWorld = scene.getDynamicsWorld();
    const auto& vbo = dynamicsWorld.getDebugDrawVbo();
    const auto count = dynamicsWorld.getDebugDrawCount();

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

void RenderPassForward::renderSceneSpaceDust(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera) {
    pipelineSpaceDust.bind(vkb);

    const auto components = scene.getView<ComponentSpaceDust>();
    if (components.begin() == components.end()) {
        return;
    }

    const auto eyesPos = camera.getEyesPos();
    if (eyesPosPrevious == Vector3{0.0f}) {
        eyesPosPrevious = eyesPos;
    }

    const auto diff = eyesPos - eyesPosPrevious;
    eyesPosPrevious = eyesPos;
    const auto speed = glm::clamp(glm::length(diff), 0.0f, 10.0f);
    const auto dir = speed > 0.0f ? glm::normalize(diff) : Vector3{0.0f};

    const float scale = 64.0f;
    const auto pos = Vector3{Vector3i{camera.getEyesPos()} / int(scale)} * Vector3{scale};

    for (auto x = -1; x <= 1; x++) {
        for (auto y = -1; y <= 1; y++) {
            for (auto z = -1; z <= 1; z++) {
                auto modelMatrix = glm::translate(Matrix4{1.0f}, pos + Vector3{x, y, z} * scale);
                modelMatrix = glm::scale(modelMatrix, Vector3{scale});

                pipelineSpaceDust.setModelMatrix(modelMatrix);
                pipelineSpaceDust.setMoveDirection(dir * speed);
                pipelineSpaceDust.flushConstants(vkb);

                pipelineSpaceDust.setTextureColor(resources.getSpaceDust());
                pipelineSpaceDust.setUniformCamera(camera.getUbo().getCurrentBuffer());
                pipelineSpaceDust.flushDescriptors(vkb);

                pipelineSpaceDust.renderMesh(vkb, resources.getMeshSpaceDust());
            }
        }
    }
}

void RenderPassForward::renderSceneShipControls(VulkanCommandBuffer& vkb, Scene& scene, const ComponentCamera& camera) {
    pipelineLines.bind(vkb);

    for (auto&& [entity, transform, shipControl] : scene.getView<ComponentTransform, ComponentShipControl>().each()) {
        pipelineLines.setModelMatrix(shipControl.getOrbitMatrix());

        if (shipControl.getApproachEntity() == NullEntity) {
            continue;
        }

        if (shipControl.getOrbitRadius() > 1.0f) {
            Matrix4 modelMatrix{1.0f};
            modelMatrix = glm::translate(modelMatrix, shipControl.getOrbitOrigin());
            modelMatrix = glm::scale(modelMatrix, Vector3{shipControl.getOrbitRadius()});
            modelMatrix *= shipControl.getOrbitMatrix();

            pipelineLines.setModelMatrix(modelMatrix);
            pipelineLines.setColor(Color4{0.0f, 0.7f, 1.0f, 0.2f});
            pipelineLines.flushConstants(vkb);

            pipelineLines.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineLines.flushDescriptors(vkb);

            pipelineLines.renderMesh(vkb, resources.getMeshOrbit());
        } else {
            const auto target = shipControl.getApproachPos();
            const auto origin = transform.getAbsolutePosition();

            auto modelMatrix = glm::lookAt(origin, target, Vector3{0.0f, 1.0f, 0.0f});
            modelMatrix = glm::inverse(modelMatrix);
            modelMatrix = glm::scale(modelMatrix, Vector3{glm::distance(target, origin)});
            // modelMatrix = glm::translate(modelMatrix, origin);

            pipelineLines.setModelMatrix(modelMatrix);
            pipelineLines.setColor(Color4{0.0f, 0.7f, 1.0f, 0.2f});
            pipelineLines.flushConstants(vkb);

            pipelineLines.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineLines.flushDescriptors(vkb);

            pipelineLines.renderMesh(vkb, resources.getMeshLineForward());
        }
    }
}

void RenderPassForward::renderSceneTacticalOverlay(VulkanCommandBuffer& vkb, Scene& scene,
                                                   const ComponentCamera& camera) {
    auto& controllerIcons = scene.getController<ControllerIcon>();

    auto* cameraOrbital = scene.tryGetComponent<ComponentCameraOrbital>(camera.getEntity());
    if (!cameraOrbital) {
        return;
    }

    IconsBufferArray buffers{};
    buffers[0] = &controllerIcons.getStaticBuffers();
    buffers[1] = &controllerIcons.getDynamicBuffers();

    if (buffers[0]->empty() && buffers[1]->empty()) {
        return;
    }

    renderSceneTacticalOverlaySpots(vkb, buffers, camera, *cameraOrbital);
    renderSceneTacticalOverlayLines(vkb, buffers, camera, *cameraOrbital);
}

void RenderPassForward::renderSceneTacticalOverlayLines(VulkanCommandBuffer& vkb, const IconsBufferArray& buffers,
                                                        const ComponentCamera& camera,
                                                        const ComponentCameraOrbital& cameraOrbital) {
    pipelineTacticalOverlayLines.bind(vkb);

    const auto modelMatrix = Matrix4{1.0f};

    pipelineTacticalOverlayLines.setModelMatrix(modelMatrix);
    pipelineTacticalOverlayLines.setColor(Color4{1.0f, 1.0f, 1.0f, 0.05f});
    pipelineTacticalOverlayLines.setPlayerPos(cameraOrbital.getTarget());
    pipelineTacticalOverlayLines.flushConstants(vkb);

    for (const auto* b : buffers) {
        for (const auto& pair : *b) {
            if (pair.second.count() == 0) {
                continue;
            }

            pipelineTacticalOverlayLines.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineTacticalOverlayLines.flushDescriptors(vkb);

            std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
            vboBindings[0] = {&pair.second.getCurrentBuffer(), 0};
            vkb.bindBuffers(vboBindings);

            vkb.draw(4, pair.second.count(), 0, 0);
        }
    }
}

void RenderPassForward::renderSceneTacticalOverlaySpots(VulkanCommandBuffer& vkb, const IconsBufferArray& buffers,
                                                        const ComponentCamera& camera,
                                                        const ComponentCameraOrbital& cameraOrbital) {
    pipelineTacticalOverlaySpots.bind(vkb);

    const auto modelMatrix = Matrix4{1.0f};

    pipelineTacticalOverlaySpots.setModelMatrix(modelMatrix);
    pipelineTacticalOverlaySpots.setColor(Color4{1.0f, 1.0f, 1.0f, 0.1f});
    pipelineTacticalOverlaySpots.setPlayerPos(cameraOrbital.getTarget());
    pipelineTacticalOverlaySpots.flushConstants(vkb);
    pipelineTacticalOverlaySpots.setScale(5.0f);

    for (const auto* b : buffers) {
        for (const auto& pair : *b) {
            if (pair.second.count() == 0) {
                continue;
            }

            pipelineTacticalOverlaySpots.setUniformCamera(camera.getUbo().getCurrentBuffer());
            pipelineTacticalOverlaySpots.flushDescriptors(vkb);

            std::array<VulkanVertexBufferBindRef, 1> vboBindings{};
            vboBindings[0] = {&pair.second.getCurrentBuffer(), 0};
            vkb.bindBuffers(vboBindings);

            vkb.draw(4, pair.second.count(), 0, 0);
        }
    }
}
