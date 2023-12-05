#pragma once

#include "../Library.hpp"
#include "GuiContext.hpp"

namespace Engine {
class ENGINE_API GuiWidget {
public:
    explicit GuiWidget(GuiContext& ctx);
    virtual ~GuiWidget() = default;

    virtual void draw() = 0;

    void setWidth(float value);
    float getWidth() const {
        return width;
    }

protected:
    GuiContext& ctx;

private:
    float width{1.0f};
};
} // namespace Engine
