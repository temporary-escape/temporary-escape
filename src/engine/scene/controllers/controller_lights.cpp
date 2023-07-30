#include "controller_lights.hpp"
#include "../scene.hpp"

using namespace Engine;

static constexpr size_t shadowMapCascadeCount = 4;

ControllerLights::ControllerLights(entt::registry& reg, Scene& scene) : reg{reg}, scene{scene} {
}

ControllerLights::~ControllerLights() = default;

void ControllerLights::update(const float delta) {
    (void)delta;
}

void ControllerLights::recalculate(VulkanRenderer& vulkan) {
    uboShadowReady = false;
    prepareUboShadow(vulkan);
    calculateShadowCamera(vulkan);
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

    const auto& viewport = camera->getViewport();
    const auto nearClip = camera->getZNear();
    const auto farClip = 5000.0f;
    const auto clipRange = farClip - nearClip;
    const auto cascadeSplitLambda = 0.96f;
    const auto nearFarMul = 4.0f;

    const auto projectionMatrix = glm::perspective(glm::radians(camera->getFov()),
                                                   static_cast<float>(viewport.x) / static_cast<float>(viewport.y),
                                                   nearClip,
                                                   farClip);

    const auto minZ = nearClip;
    const auto maxZ = nearClip + clipRange;

    const auto range = maxZ - minZ;
    const auto ratio = maxZ / minZ;

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    for (size_t i = 0; i < shadowMapCascadeCount; i++) {
        float p = static_cast<float>(i + 1) / static_cast<float>(shadowMapCascadeCount);
        float log = minZ * std::pow(ratio, p);
        float uniform = minZ + range * p;
        float d = cascadeSplitLambda * (log - uniform) + uniform;
        shadowsViewProj.cascadeSplits[i] = (d - nearClip) / clipRange;
    }

    auto systemDirLights = reg.view<ComponentTransform, ComponentDirectionalLight>();
    if (systemDirLights.begin() == systemDirLights.end()) {
        return;
    }

    // Pick first directional lights
    // This should be the sun
    const auto&& [entity, transform, light] = *systemDirLights.each().begin();

    const auto lightDir = -glm::normalize(transform.getAbsolutePosition());

    // Calculate orthographic projection matrix for each cascade
    float lastSplitDist = 0.0;
    for (size_t c = 0; c < shadowMapCascadeCount; c++) {
        const auto splitDist = shadowsViewProj.cascadeSplits[c];

        glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, -1.0f, 1.0f),
            glm::vec3(-1.0f, -1.0f, 1.0f),
        };

        // Project frustum corners into world space
        const auto invCam = glm::inverse(projectionMatrix * camera->getViewMatrix());
        for (size_t i = 0; i < 8; i++) {
            const auto invCorner = invCam * Vector4(frustumCorners[i], 1.0f);
            frustumCorners[i] = invCorner / invCorner.w;
        }

        for (size_t i = 0; i < 4; i++) {
            const auto dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        auto frustumCenter = camera->getEyesPos();
        for (size_t i = 0; i < 8; i++) {
            frustumCenter += frustumCorners[i];
        }
        frustumCenter /= 8.0f;

        float radius = 0.0f;
        for (size_t i = 0; i < 8; i++) {
            float distance = glm::length(frustumCorners[i] - frustumCenter);
            radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        const auto maxExtents = glm::vec3(radius);
        const auto minExtents = -maxExtents;

        Matrix4 cameraModel{1.0f};
        Camera shadowCamera{cameraModel};

        shadowCamera.lookAt(frustumCenter - lightDir * -minExtents.z * nearFarMul, frustumCenter, {0.0f, 1.0f, 0.0f});

        shadowCamera.setProjectionMatrix(glm::orthoLH_ZO(
            minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, (maxExtents.z - minExtents.z) * nearFarMul));

        shadowsViewProj.lightMat[c] = shadowCamera.getProjectionMatrix() * shadowCamera.getViewMatrix();
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
