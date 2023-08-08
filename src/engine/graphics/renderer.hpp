#pragma once

#include "../utils/exceptions.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include "render_pass.hpp"
#include "render_resources.hpp"

namespace Engine {
class ENGINE_API Scene;

class ENGINE_API Renderer {
public:
    explicit Renderer(const RenderOptions& options, VulkanRenderer& vulkan);
    virtual ~Renderer() = default;

    void render(VulkanCommandBuffer& vkb, Scene& scene, const Vector2i& viewport);
    void blit(VulkanCommandBuffer& vkb);

    const RenderOptions& getOptions() const {
        return options;
    }

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
    RenderOptions options;
    VulkanRenderer& vulkan;
    std::vector<std::unique_ptr<RenderPass>> passes;
};
} // namespace Engine
