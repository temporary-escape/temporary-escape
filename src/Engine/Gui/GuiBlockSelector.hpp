#pragma once

#include "../Assets/Block.hpp"
#include "GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiBlockSelector : public GuiWindow {
public:
    explicit GuiBlockSelector(Nuklear& nuklear);

    ~GuiBlockSelector() override = default;

private:
    void drawLayout() override;

protected:
    void beforeDraw(const Vector2i& viewport) override;
};
} // namespace Engine
