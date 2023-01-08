#pragma once

#include "../math/matrix.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include <functional>
#include <list>

namespace Engine {
class ENGINE_API ShaderModules {
public:
    using LoadQueue = std::list<std::function<void()>>;

    explicit ShaderModules(const Config& config, VulkanRenderer& vulkan);

    VulkanShaderModule& findByName(const std::string& name);

    LoadQueue& getLoadQueue() {
        return loadQueue;
    }

private:
    std::unordered_map<std::string, VulkanShaderModule> modules;
    LoadQueue loadQueue;
};

class ENGINE_API Shader {
public:
    struct CameraUniform {
        Matrix4 transformationProjectionMatrix;
        Matrix4 viewProjectionInverseMatrix;
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Vector2i viewport;
        float padding0[2];
        Vector3 eyesPos;
        float padding1[1];
    };

    Shader() = default;
    virtual ~Shader() = default;
    Shader(const Shader& other) = delete;
    Shader(Shader&& other) = default;
    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(Shader&& other) = default;

    operator bool() const {
        return !!pipeline && !!descriptorSetLayout;
    }

    VulkanPipeline& getPipeline() {
        return pipeline;
    }

    const VulkanPipeline& getPipeline() const {
        return pipeline;
    }

    VulkanDescriptorSetLayout& getDescriptorSetLayout() {
        return descriptorSetLayout;
    }

    const VulkanDescriptorSetLayout& getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }

protected:
    VulkanDescriptorSetLayout descriptorSetLayout;
    VulkanPipeline pipeline;
};
} // namespace Engine
