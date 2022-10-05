#include "GuiWindow.hpp"

using namespace Engine;

GuiWindow::GuiWindow(Nuklear& nuklear) : nuklear{nuklear} {
}

void GuiWindow::draw(const Vector2i& viewport) {
    beforeDraw(viewport);
    if (nuklear.beginWindow(title, pos, size, flags)) {
        drawLayout();
        nuklear.endWindow();
    }
}

const Vector2& GuiWindow::getSize() const {
    return size;
}

const Vector2& GuiWindow::getPos() const {
    return pos;
}

Nuklear::Flags GuiWindow::getFlags() const {
    return flags;
}

void GuiWindow::setSize(const Vector2& size) {
    GuiWindow::size = size;
}

void GuiWindow::setPos(const Vector2& pos) {
    GuiWindow::pos = pos;
}

void GuiWindow::setFlags(unsigned int flags) {
    GuiWindow::flags = flags;
}

void GuiWindow::beforeDraw(const Vector2i& viewport) {
    (void)viewport;
}

const std::string& GuiWindow::getTitle() const {
    return title;
}

void GuiWindow::setTitle(const std::string& title) {
    GuiWindow::title = title;
    flags = flags | Nuklear::WindowFlags::Title;
}

void GuiWindow::setBordered() {
    flags = flags | Nuklear::WindowFlags::Border;
}

void GuiWindow::setWithBackground() {
    flags = flags | Nuklear::WindowFlags::Background;
}

void GuiWindow::setNoScrollbar() {
    flags = flags | Nuklear::WindowFlags::NoScrollbar;
}

void GuiWindow::setDynamic() {
    flags = flags | Nuklear::WindowFlags::Dynamic;
}
