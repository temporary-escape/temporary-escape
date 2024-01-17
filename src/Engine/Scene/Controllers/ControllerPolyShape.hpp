#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerPolyShape : public Controller {
public:
    explicit ControllerPolyShape(Scene& scene, entt::registry& reg);
    ~ControllerPolyShape() override;
    NON_COPYABLE(ControllerPolyShape);
    NON_MOVEABLE(ControllerPolyShape);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    Scene& scene;
    entt::registry& reg;
};
} // namespace Engine
