#pragma once

#include "../Library.hpp"
#include "GuiContext.hpp"

namespace Engine {
class ENGINE_API GuiWidget {
public:
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

protected:
    GuiContext& ctx;

private:
    virtual void drawInternal();

    float width{1.0f};
    bool widthPixels{false};
    std::string tooltip;
};
} // namespace Engine
