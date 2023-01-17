#pragma once

#include "../graphics/nuklear.hpp"
#include "gui/gui_context_menu.hpp"

namespace Engine {
class Gui : public NuklearWindow {
public:
    struct ButtonItem {
        std::string label;
        std::function<void()> callback;
    };

    explicit Gui(const Config& config);

    void draw(Nuklear& nuklear, const Vector2& viewport) override;

    GuiContextMenu contextMenu;

private:
    const Config& config;
};
} // namespace Engine
