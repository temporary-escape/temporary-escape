#include "GuiWindow.hpp"
#include "GuiContext.hpp"

using namespace Scissio;

GuiWindow::GuiWindow(GuiContext& ctx, std::string title) : ctx(ctx), title(std::move(title)) {
    ctx.addWindow(*this);
}

GuiWindow::~GuiWindow() {
    ctx.removeWindow(*this);
}
