#include "gui_modal_loading.hpp"

using namespace Engine;

GuiModalLoading::GuiModalLoading(const std::string& title) : progress{0.5f} {
    setSize({300.0f, 100.0f});
    setFlags(Nuklear::WindowFlags::Dynamic | Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border |
             Nuklear::WindowFlags::Title | Nuklear::WindowFlags::NoInput);
    setTitle(title);
}

void GuiModalLoading::drawLayout(Nuklear& nuklear) {
    nuklear.layoutDynamic(25.0f, 1);
    nuklear.label("Loading...");
    nuklear.layoutDynamic(25.0f, 1);
    nuklear.progress(progress);
}

void GuiModalLoading::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - 50});
}
