#pragma once

#include "GuiWidgetLayout.hpp"

namespace Engine {
template <typename T> class ENGINE_API GuiWidgetSelectableGroup : public GuiWidgetLayout {
public:
    using OnClickCallback = std::function<void()>;

    explicit GuiWidgetSelectableGroup(GuiContext& ctx, T& shared, const T& value) :
        GuiWidgetLayout{ctx}, shared{shared}, value{value}, name{std::to_string(reinterpret_cast<uint64_t>(this))} {
    }

    void draw() override {
        const auto hovered = ctx.isHovered();
        const auto mouseDown = ctx.isMouseDown(MouseButton::Left);

        const auto selected = shared == value;

        if (hovered && mouseDown && !selected) {
            shared = value;
            if (onClick) {
                onClick();
            }
        }

        if (ctx.groupBegin(name, false, true, hovered || selected ? Colors::primary : Colors::white)) {
            GuiWidgetLayout::drawInternal();
            ctx.groupEnd();
        }
    }
    void setOnClick(OnClickCallback value) {
        onClick = std::move(value);
    }

private:
    T& shared;
    T value;
    std::string name;
    OnClickCallback onClick;
};
} // namespace Engine
