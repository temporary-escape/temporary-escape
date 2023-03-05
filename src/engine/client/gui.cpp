#include "gui.hpp"

using namespace Engine;

Gui::Gui(const Config& config) : config{config} {
    contextMenu.setEnabled(false);
    blockSelector.setEnabled(false);
}

void Gui::draw(Nuklear& nuklear, const Vector2& viewport) {
    contextMenu.draw(nuklear, viewport);
    blockSelector.draw(nuklear, viewport);
}
