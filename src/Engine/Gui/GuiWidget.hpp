#pragma once

#include "../Library.hpp"
#include "GuiContext.hpp"

namespace Engine {
class ENGINE_API GuiWidget {
public:
    using OnHoverCallback = std::function<void(bool)>;

    explicit GuiWidget(GuiContext& ctx);
    virtual ~GuiWidget() = default;

    virtual void draw();

    void setWidth(float value, bool pixels = false);
    float getWidth() const {
        return width;
    }
    bool isWidthPixels() const {
        return widthPixels;
    }
    void setTooltip(std::string value);
    void setOnHover(OnHoverCallback value);

protected:
    virtual void beforeDraw() {
    }
    virtual void drawInternal();

    GuiContext& ctx;

private:
    float width{1.0f};
    bool widthPixels{false};
    std::string tooltip;
    OnHoverCallback onHover;
    bool hovered{false};
};
} // namespace Engine
