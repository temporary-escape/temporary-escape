#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockSelector : public GuiWindow {
public:
    explicit GuiBlockSelector(Nuklear& nuklear);

    ~GuiBlockSelector() override = default;

private:
    void drawLayout() override;
    void beforeDraw(const Vector2i& viewport) override;
};
} // namespace Engine
