#pragma once

#include "../../assets/image.hpp"
#include "../component.hpp"

namespace Engine {
class ENGINE_API Component2DSelectable : public Component {
public:
    Component2DSelectable() = default;
    explicit Component2DSelectable(entt::registry& reg, entt::entity handle, const Vector2& size);
    virtual ~Component2DSelectable() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(Component2DSelectable);

    const Vector2& getSize() const {
        return size;
    }

    void setSize(const Vector2& value) {
        size = value;
    }

    const std::function<void(bool)>& getOnHoverCallback() const {
        return onHoverCallback;
    }

    const std::function<void(MouseButton)>& getOnSelectCallback() const {
        return onSelectCallback;
    }

    template <typename Fn> void setOnHoverCallback(Fn&& fn) {
        onHoverCallback = std::forward<Fn>(fn);
    }

    template <typename Fn> void setOnSelectCallback(Fn&& fn) {
        onSelectCallback = std::forward<Fn>(fn);
    }

private:
    Vector2 size;
    std::function<void(MouseButton)> onSelectCallback;
    std::function<void(bool)> onHoverCallback;
};
} // namespace Engine
