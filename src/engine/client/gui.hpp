#pragma once

#include "../graphics/nuklear.hpp"
#include "gui/gui_block_selector.hpp"
#include "gui/gui_context_menu.hpp"

namespace Engine {
class Gui : public NuklearWindow {
public:
    explicit Gui(const Config& config);

    void draw(Nuklear& nuklear, const Vector2& viewport) override;

    GuiContextMenu contextMenu;
    GuiBlockSelector blockSelector;

private:
    const Config& config;
};
} // namespace Engine
