#include "gui_window.hpp"

using namespace Engine;

GuiWindow::GuiWindow() : title{std::to_string(reinterpret_cast<uint64_t>(this))} {
}

void GuiWindow::draw(Nuklear& nuklear, const Vector2& viewport) {
    if (!enabled) {
        return;
    }

    if (fontSize) {
        nuklear.fontSize(fontSize);
    }

    beforeDraw(viewport);
    if (nuklear.beginWindow(title, pos, size, flags)) {
        drawLayout(nuklear);
        nuklear.endWindow();
    }

    if (fontSize) {
        nuklear.resetFont();
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

void GuiWindow::setSize(const Vector2& value) {
    size = value;
}

void GuiWindow::setPos(const Vector2& value) {
    pos = value;
}

void GuiWindow::setFlags(unsigned int value) {
    flags = value;
}

void GuiWindow::setEnabled(bool value) {
    enabled = value;
}

void GuiWindow::beforeDraw(const Vector2& viewport) {
    (void)viewport;
}

const std::string& GuiWindow::getTitle() const {
    return title;
}

void GuiWindow::setTitle(const std::string& value) {
    title = value;
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

void GuiWindow::setFontSize(const int size) {
    fontSize = size;
}

bool GuiWindow::isCursorInside(const Vector2i& mousePos) {
    return mousePos.x > pos.x && mousePos.x < pos.x + size.x && mousePos.y > pos.y && mousePos.y < pos.y + size.y;
}