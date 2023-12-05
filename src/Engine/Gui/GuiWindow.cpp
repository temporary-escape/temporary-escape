#include "GuiWindow.hpp"

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

    beforeDraw(nuklear, viewport);
    if (nuklear.beginWindow(title, pos, size, flags)) {
        pos = nuklear.getWindowPos();
        drawLayout(nuklear);
    } else if (getFlags() & static_cast<Nuklear::Flags>(Nuklear::WindowFlags::Closeable)) {
        setEnabled(false);
    }
    nuklear.endWindow();

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

void GuiWindow::beforeDraw(Nuklear& nuklear, const Vector2& viewport) {
    (void)nuklear;
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

void GuiWindow::setAlwaysBackground() {
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

GuiWindow2::GuiWindow2(const FontFamily& fontFamily, const int fontSize) :
    GuiContext{fontFamily, fontSize},
    GuiWidgetLayout{static_cast<GuiContext&>(*this)},
    flags{WindowFlag::Title | WindowFlag::NoScrollbar} {
}

void GuiWindow2::update(const Vector2i& viewport) {
    GuiContext::update();
    setPos(Vector2{viewport} / 2.0f - size / 2.0f);
}

void GuiWindow2::draw() {
    // We are rendering into an FBO, therefore the position is always at [0, 0]
    if (GuiContext::beginWindow(title, {0.0f, 0.0f}, size, flags)) {
        GuiWidgetLayout::draw();
    }
    GuiContext::endWindow();
}

void GuiWindow2::setTitle(std::string value) {
    title = std::move(value);
}

void GuiWindow2::setSize(const Vector2& value) {
    size = value;
}

void GuiWindow2::setPos(const Vector2& value) {
    pos = value;
}

void GuiWindow2::setEnabled(const bool value) {
    enabled = value;
}

void GuiWindow2::setBordered(bool value) {
    if (value) {
        flags |= WindowFlag::Border;
    } else {
        flags &= ~WindowFlag::Border;
    }
}

void GuiWindow2::setBackground(bool value) {
    if (value) {
        flags |= WindowFlag::Background;
    } else {
        flags &= ~WindowFlag::Background;
    }
}

void GuiWindow2::setNoScrollbar(bool value) {
    if (value) {
        flags |= WindowFlag::NoScrollbar;
    } else {
        flags &= ~WindowFlag::NoScrollbar;
    }
}

void GuiWindow2::setDynamic(bool value) {
    if (value) {
        flags |= WindowFlag::Dynamic;
    } else {
        flags &= ~WindowFlag::Dynamic;
    }
}
