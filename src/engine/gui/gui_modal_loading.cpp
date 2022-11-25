#include "gui_modal_loading.hpp"

using namespace Engine;

GuiModalLoading::GuiModalLoading(Nuklear& nuklear, const std::string& title) : GuiWindow{nuklear}, progress{0.5f} {
    setSize({300.0f, 100.0f});
    setFlags(Nuklear::WindowFlags::Dynamic | Nuklear::WindowFlags::NoScrollbar | Nuklear::WindowFlags::Border |
             Nuklear::WindowFlags::Title | Nuklear::WindowFlags::NoInput);
    setTitle(title);
}

void GuiModalLoading::drawLayout() {
    nuklear.layoutDynamic(25.0f, 1);
    nuklear.label("Loading...");
    nuklear.layoutDynamic(25.0f, 1);
    nuklear.progress(progress);
}

void GuiModalLoading::beforeDraw(const Vector2i& viewport) {
    setPos({viewport.x / 2 - getSize().x / 2, viewport.y / 2 - 50});
}
