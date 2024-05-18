#include "GuiWindow.hpp"

using namespace Engine;

GuiWindow::GuiWindow(GuiContext& ctx, const FontFamily& fontFamily, const int fontSize) :
    GuiWidgetLayout{ctx},
    ctx{ctx},
    id{std::to_string(reinterpret_cast<uint64_t>(this))},
    flags{GuiContext::WindowFlag::Title | GuiContext::WindowFlag::NoScrollbar | GuiContext::WindowFlag::Border} {
}

void GuiWindow::update(const Vector2i& viewport) {
    if (centered) {
        setPos(Vector2{viewport} / 2.0f - size / 2.0f);
    }
}

void GuiWindow::draw() {
    // We are rendering into an FBO, therefore the position is always at [0, 0]
    GuiContext::WindowOptions options{
        flags,
        opacity,
    };

    if (ignoreInput == 0) {
        ctx.setPadding(padding);
        if (ctx.windowBegin(id, title, pos, size, options)) {
            GuiWidgetLayout::draw();
        } else {
            close();
        }
        ctx.windowEnd(flags);
    }

    if (ignoreInput > 0) {
        --ignoreInput;
    }
}

void GuiWindow::close() {
    setEnabled(false);
    if (onCloseCallback) {
        onCloseCallback();
    }
}

void GuiWindow::setOnClose(OnCloseCallback value) {
    onCloseCallback = std::move(value);
}

void GuiWindow::setTitle(const std::string_view& value) {
    title = fmt::format("<b>{}</b>", value);
}

void GuiWindow::setSize(const Vector2& value) {
    size = value;
}

void GuiWindow::setPos(const Vector2& value) {
    pos = value;
}

void GuiWindow::setEnabled(const bool value) {
    if (value) {
        ignoreInput = 1;
    }
    enabled = value;
}

void GuiWindow::setCentered(const bool value) {
    centered = value;
}

void GuiWindow::setOpacity(const float value) {
    opacity = value;
}

void GuiWindow::setBordered(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::Border;
    } else {
        flags &= ~GuiContext::WindowFlag::Border;
    }
}

void GuiWindow::setBackground(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::Background;
    } else {
        flags &= ~GuiContext::WindowFlag::Background;
    }
}

void GuiWindow::setNoScrollbar(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::NoScrollbar;
    } else {
        flags &= ~GuiContext::WindowFlag::NoScrollbar;
    }
}

void GuiWindow::setHeader(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::Title;
    } else {
        flags &= ~GuiContext::WindowFlag::Title;
    }
}

void GuiWindow::setDynamic(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::Dynamic;
    } else {
        flags &= ~GuiContext::WindowFlag::Dynamic;
    }
}

void GuiWindow::setNoInput(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::NoInput;
    } else {
        flags &= ~GuiContext::WindowFlag::NoInput;
    }
}

void GuiWindow::setHeaderPrimary(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::HeaderPrimary;
    } else {
        flags &= ~GuiContext::WindowFlag::HeaderPrimary;
    }
}

void GuiWindow::setHeaderSuccess(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::HeaderSuccess;
    } else {
        flags &= ~GuiContext::WindowFlag::HeaderSuccess;
    }
}

void GuiWindow::setHeaderDanger(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::HeaderDanger;
    } else {
        flags &= ~GuiContext::WindowFlag::HeaderDanger;
    }
}

void GuiWindow::setCloseable(const bool value) {
    if (value) {
        flags |= GuiContext::WindowFlag::Closeable;
    } else {
        flags &= ~GuiContext::WindowFlag::Closeable;
    }
}
