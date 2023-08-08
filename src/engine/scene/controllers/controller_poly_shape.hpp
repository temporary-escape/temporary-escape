#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerPolyShape : public Controller {
public:
    explicit ControllerPolyShape(entt::registry& reg);
    ~ControllerPolyShape() override;
    NON_COPYABLE(ControllerPolyShape);
    NON_MOVEABLE(ControllerPolyShape);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    entt::registry& reg;
};
} // namespace Engine
