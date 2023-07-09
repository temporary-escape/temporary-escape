#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerModel : public Controller {
public:
    struct ModelMatrixBuffer {
        VulkanDoubleBuffer vboModels;
        VulkanDoubleBuffer vboEntityColors;
        size_t capacity{0};
        size_t count{0};
        bool expand{true};
    };

    explicit ControllerModel(entt::registry& reg);
    ~ControllerModel() override;

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    const std::unordered_map<Model*, ModelMatrixBuffer>& getBuffers() const {
        return buffers;
    }

private:
    ModelMatrixBuffer& getBufferFor(const ModelPtr& model, VulkanRenderer& vulkan);

    entt::registry& reg;
    std::unordered_map<Model*, ModelMatrixBuffer> buffers;
};
} // namespace Engine
