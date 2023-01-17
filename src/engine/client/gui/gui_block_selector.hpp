#pragma once

#include "../../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiBlockSelector : public GuiWindow {
public:
    explicit GuiBlockSelector();

    ~GuiBlockSelector() override = default;

private:
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(const Vector2& viewport) override;
};
} // namespace Engine
