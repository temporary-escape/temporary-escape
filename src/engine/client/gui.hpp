#pragma once

#include "../graphics/nuklear.hpp"
#include "gui/gui_block_action_bar.hpp"
#include "gui/gui_block_selector.hpp"
#include "gui/gui_context_menu.hpp"

namespace Engine {
class Gui : public NuklearWindow {
public:
    struct Preferences {
        GuiBlockActionBar::Preferences blockActionBar;

        YAML_DEFINE(blockActionBar);
    };

    explicit Gui(const Config& config, Registry& registry);
    ~Gui() noexcept;

    void draw(Nuklear& nuklear, const Vector2& viewport) override;

private:
    const Config& config;
    Preferences preferences;

public:
    GuiContextMenu contextMenu;
    GuiBlockSelector blockSelector;
    GuiBlockActionBar blockActionBar;
};
} // namespace Engine
