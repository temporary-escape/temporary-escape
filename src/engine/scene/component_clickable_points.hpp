#pragma once

#include "../assets/image.hpp"
#include "component.hpp"
#include "component_user_input.hpp"

namespace Engine {
class ENGINE_API ComponentClickablePoints : public Component, public UserInput {
public:
    using Point = Vector4;
    using Output = Vector2;

    ComponentClickablePoints() = default;
    virtual ~ComponentClickablePoints() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentClickablePoints);

    void add(const Vector3& pos);
    void clear();
    void recalculate(VulkanRenderer& vulkan);
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    bool isReady() const {
        return sboInput && sboOutput;
    }

    void setOnHoverCallback(std::function<void(size_t)> fn) {
        onHoverCallback = std::move(fn);
    }
    void setOnBlurCallback(std::function<void()> fn) {
        onBlurCallback = std::move(fn);
    }
    void setOnClickCallback(std::function<void(size_t, bool, MouseButton)> fn) {
        onClickCallback = std::move(fn);
    }

    Vector2 getOutputPoint(size_t idx) const;

    const VulkanBuffer& getBufferInput() const {
        return sboInput;
    }

    const VulkanBuffer& getBufferOutput() const {
        return sboOutput.getCurrentBuffer();
    }

    size_t getCount() const {
        return points.size();
    }

private:
    std::optional<size_t> findNearestPoint(const Vector2i& pos);

    std::vector<Point> points;
    Vector2 size{32.0f, 32.0f};
    VulkanBuffer sboInput;
    VulkanDoubleBuffer sboOutput;

    std::function<void(size_t)> onHoverCallback;
    std::function<void()> onBlurCallback;
    std::function<void(size_t, bool, MouseButton)> onClickCallback;
    bool blur{false};
};
} // namespace Engine
