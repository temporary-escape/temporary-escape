#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerIconSelectable : public Controller, public UserInput {
public:
    struct ALIGNED(16) Input {
        Vector4 position;
        alignas(8) Vector2 size;
        alignas(4) uint32_t id;
    };

    static_assert(sizeof(Input) == 32);

    struct ALIGNED(16) Output {
        Vector4 position;
        alignas(8) Vector2 size;
        alignas(4) uint32_t id;
    };

    static_assert(sizeof(Output) == 32);

    explicit ControllerIconSelectable(entt::registry& reg);
    ~ControllerIconSelectable() override;
    NON_COPYABLE(ControllerIconSelectable);
    NON_MOVEABLE(ControllerIconSelectable);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanArrayBuffer& getStaticBufferInput() const {
        return staticBufferInput;
    }

    const VulkanDoubleBuffer& getStaticBufferOutput() const {
        return staticBufferOutput;
    }

    const VulkanArrayBuffer& getDynamicBufferInput() const {
        return dynamicBufferInput;
    }

    const VulkanDoubleBuffer& getDynamicBufferOutput() const {
        return dynamicBufferOutput;
    }

    const std::optional<entt::entity>& getSelected() const {
        return selectedEntity;
    }

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

private:
    static void prepareOutput(VulkanRenderer& vulkan, VulkanDoubleBuffer& buffer, size_t count);
    std::optional<Output> findNearestPoint(VulkanBuffer& buffer, size_t count);

    void addOrUpdate(entt::entity handle, const ComponentTransform& transform, const ComponentIcon& icon);
    void remove(entt::entity handle);

    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    VulkanArrayBuffer staticBufferInput;
    VulkanArrayBuffer dynamicBufferInput;
    VulkanDoubleBuffer staticBufferOutput;
    VulkanDoubleBuffer dynamicBufferOutput;
    Vector2 mousePos;
    std::optional<entt::entity> selectedEntity;
};
} // namespace Engine
