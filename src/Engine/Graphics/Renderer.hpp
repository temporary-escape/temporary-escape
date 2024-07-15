#pragma once

#include "../Utils/Exceptions.hpp"
#include "../Vulkan/VulkanRenderer.hpp"
#include "RenderPass.hpp"
#include "RenderResources.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API Renderer {
public:
    explicit Renderer(VulkanRenderer& vulkan);
    virtual ~Renderer() = default;
    NON_MOVEABLE(Renderer);
    NON_COPYABLE(Renderer);

    virtual void render(VulkanCommandBuffer& vkb, VulkanCommandBuffer& vkbc, Scene& scene);

protected:
    void addRenderPass(std::unique_ptr<RenderPass> pass);
    template <typename T> T& getRenderPass() {
        for (auto& pass : passes) {
            auto& p = *pass;
            if (typeid(p).hash_code() == typeid(T).hash_code()) {
                return *dynamic_cast<T*>(pass.get());
            }
        }
        EXCEPTION("No such render pass of type: {}", typeid(T).name());
    }
    void create();

private:
    VulkanRenderer& vulkan;
    std::vector<std::unique_ptr<RenderPass>> passes;
};
} // namespace Engine
