#include "ControllerLights.hpp"
#include "../Scene.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static constexpr size_t shadowMapCascadeCount = 4;

ControllerLights::ControllerLights(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerLights::~ControllerLights() = default;

void ControllerLights::update(const float delta) {
    (void)delta;
}

void ControllerLights::recalculate(VulkanRenderer& vulkan) {
    uboShadowReady = false;
    prepareUboShadow(vulkan);
    calculateShadowCamera(vulkan);
    updateDirectionalLights(vulkan);
}

void ControllerLights::updateDirectionalLights(VulkanRenderer& vulkan) {
    DirectionalLightsUniform uniform{};

    auto system = scene.getView<ComponentTransform, ComponentDirectionalLight>();
    for (auto&& [entity, transform, light] : system.each()) {
        uniform.colors[uniform.count] = light.getColor();
        uniform.directions[uniform.count] = Vector4{transform.getPosition(), 0.0f};

        uniform.count++;
        if (uniform.count >= sizeof(DirectionalLightsUniform::colors) / sizeof(Vector4)) {
            break;
        }
    }

    if (!uboDirectionalLights) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(DirectionalLightsUniform);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        bufferInfo.memoryFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        uboDirectionalLights = vulkan.createDoubleBuffer(bufferInfo);
    }

    uboDirectionalLights.subDataLocal(&uniform, 0, sizeof(uniform));
}

// Based on:
// https://github.com/SaschaWillems/Vulkan/blob/1ba7e58f9336a796fef629a3b3aa296eb762ec76/examples/shadowmappingcascade/shadowmappingcascade.cpp#L708
void ControllerLights::calculateShadowCamera(VulkanRenderer& vulkan) {
    const auto* camera = scene.getPrimaryCamera();
    if (!camera) {
        return;
    }

    std::array<Camera::Uniform, 4> uniforms{};
    ShadowsViewProj shadowsViewProj{};

    const auto& viewport = Vector2d{camera->getViewport()};
    const auto nearClip = static_cast<double>(camera->getZNear());
    const auto farClip = static_cast<double>(camera->getZFar());
    const auto clipRange = farClip - nearClip;
    const auto cascadeSplitLambda = 0.95;
    const auto nearFarMul = 1.0;
    const auto eyesPos = Vector3d{camera->getEyesPos()};

    const auto projectionMatrix = Matrix4d{
        glm::perspective<double>(glm::radians(camera->getFov()),
                                 static_cast<double>(viewport.x) / static_cast<double>(viewport.y),
                                 nearClip,
                                 farClip),
    };

    const auto viewMatrix = Matrix4d{camera->getViewMatrix()};

    const auto minZ = nearClip;
    const auto maxZ = nearClip + clipRange;

    const auto range = maxZ - minZ;
    const auto ratio = maxZ / minZ;

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    for (size_t i = 0; i < shadowMapCascadeCount; i++) {
        double p = static_cast<double>(i + 1) / static_cast<double>(shadowMapCascadeCount);
        double log = minZ * std::pow(ratio, p);
        double uniform = minZ + range * p;
        double d = cascadeSplitLambda * (log - uniform) + uniform;
        shadowsViewProj.cascadeSplits[i] = static_cast<float>((d - nearClip) / clipRange);
    }

    auto systemDirLights = reg.view<ComponentTransform, ComponentDirectionalLight>();
    if (systemDirLights.begin() == systemDirLights.end()) {
        return;
    }

    // Pick first directional lights
    // This should be the sun
    const auto&& [entity, transform, light] = *systemDirLights.each().begin();

    const auto lightDir = Vector3d{-glm::normalize(transform.getAbsolutePosition())};

    // Calculate orthographic projection matrix for each cascade
    double lastSplitDist = 0.0;
    for (size_t c = 0; c < shadowMapCascadeCount; c++) {
        const auto splitDist = shadowsViewProj.cascadeSplits[c];

        Vector3d frustumCorners[8] = {
            {-1.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f},
            {-1.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f},
            {1.0f, -1.0f, 1.0f},
            {-1.0f, -1.0f, 1.0f},
        };

        // Project frustum corners into world space
        const auto invCam = glm::inverse(projectionMatrix * viewMatrix);
        for (size_t i = 0; i < 8; i++) {
            const auto invCorner = invCam * Vector4(frustumCorners[i], 1.0f);
            frustumCorners[i] = invCorner / invCorner.w;
        }

        for (size_t i = 0; i < 4; i++) {
            const auto dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * static_cast<double>(splitDist));
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        auto frustumCenter = Vector3d{0.0};
        for (size_t i = 0; i < 8; i++) {
            frustumCenter += frustumCorners[i];
        }
        frustumCenter /= 8.0;

        double radius = 0.0;
        for (size_t i = 0; i < 8; i++) {
            double distance = glm::length(frustumCorners[i] - frustumCenter);
            radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0) / 16.0;

        const auto maxExtents = Vector3d(radius);
        const auto minExtents = -maxExtents;

        Matrix4 cameraModel{1.0f};
        Camera shadowCamera{cameraModel};

        auto view = glm::lookAt(eyesPos - lightDir * -minExtents.z, eyesPos, {0.0f, 1.0f, 0.0f});
        shadowCamera.setViewMatrix(view);

        auto proj = glm::orthoLH_ZO(
            minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0, (maxExtents.z - minExtents.z) * nearFarMul);
        shadowCamera.setProjectionMatrix(proj);

        shadowsViewProj.lightMat[c] = proj * view;
        shadowsViewProj.cascadeSplits[c] = nearClip + splitDist * clipRange;

        uniforms[c] = shadowCamera.createUniform(false);
    }

    uboShadowCamera.subDataLocal(uniforms.data(), 0, sizeof(Camera::Uniform) * uniforms.size());
    uboShadowsViewProj.subDataLocal(&shadowsViewProj, 0, sizeof(ShadowsViewProj));

    uboShadowReady = true;
}

void ControllerLights::prepareUboShadow(VulkanRenderer& vulkan) {
    if (!uboShadowCamera) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(Camera::Uniform) * 4;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        uboShadowCamera = vulkan.createDoubleBuffer(bufferInfo);
    }

    if (!uboShadowsViewProj) {
        VulkanBuffer::CreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(ShadowsViewProj);
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferInfo.memoryFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        uboShadowsViewProj = vulkan.createDoubleBuffer(bufferInfo);
    }
}
