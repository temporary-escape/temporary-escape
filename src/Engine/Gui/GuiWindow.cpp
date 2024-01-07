#include "GuiWindow.hpp"

using namespace Engine;

GuiWindow::GuiWindow(const FontFamily& fontFamily, const int fontSize) :
    GuiContext{fontFamily, fontSize},
    GuiWidgetLayout{static_cast<GuiContext&>(*this)},
    id{std::to_string(reinterpret_cast<uint64_t>(this))},
    flags{WindowFlag::Title | WindowFlag::NoScrollbar} {
}

void GuiWindow::update(const Vector2i& viewport) {
    if (centered) {
        setPos(Vector2{viewport} / 2.0f - size / 2.0f);
    }
    GuiContext::update();
}

void GuiWindow::draw() {
    // We are rendering into an FBO, therefore the position is always at [0, 0]
    if (GuiContext::windowBegin(id, title, {0.0f, 0.0f}, size, flags)) {
        GuiWidgetLayout::draw();
    }
    GuiContext::windowEnd(flags);
}

void GuiWindow::setTitle(std::string value) {
    title = std::move(value);
}

void GuiWindow::setSize(const Vector2& value) {
    size = value;
}

void GuiWindow::setPos(const Vector2& value) {
    pos = value;
}

void GuiWindow::setEnabled(const bool value) {
    enabled = value;
    setDirty();
}

void GuiWindow::setCentered(const bool value) {
    centered = value;
}

void GuiWindow::setBordered(const bool value) {
    if (value) {
        flags |= WindowFlag::Border;
    } else {
        flags &= ~WindowFlag::Border;
    }
}

void GuiWindow::setBackground(const bool value) {
    if (value) {
        flags |= WindowFlag::Background;
    } else {
        flags &= ~WindowFlag::Background;
    }
}

void GuiWindow::setNoScrollbar(const bool value) {
    if (value) {
        flags |= WindowFlag::NoScrollbar;
    } else {
        flags &= ~WindowFlag::NoScrollbar;
    }
}

void GuiWindow::setHeader(const bool value) {
    if (value) {
        flags |= WindowFlag::Title;
    } else {
        flags &= ~WindowFlag::Title;
    }
}

void GuiWindow::setTransparent(const bool value) {
    if (value) {
        flags |= WindowFlag::Transparent;
    } else {
        flags &= ~WindowFlag::Transparent;
    }
}

void GuiWindow::setDynamic(const bool value) {
    if (value) {
        flags |= WindowFlag::Dynamic;
    } else {
        flags &= ~WindowFlag::Dynamic;
    }
}

void GuiWindow::setNoInput(const bool value) {
    if (value) {
        flags |= WindowFlag::NoInput;
    } else {
        flags &= ~WindowFlag::NoInput;
    }
}

void GuiWindow::setHeaderSuccess(const bool value) {
    if (value) {
        flags |= WindowFlag::HeaderSuccess;
    } else {
        flags &= ~WindowFlag::HeaderSuccess;
    }
}

void GuiWindow::setHeaderDanger(const bool value) {
    if (value) {
        flags |= WindowFlag::HeaderDanger;
    } else {
        flags &= ~WindowFlag::HeaderDanger;
    }
}
