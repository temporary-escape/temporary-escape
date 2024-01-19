#include "GuiWindowShipToolbar.hpp"
#include "../../Assets/AssetsManager.hpp"

using namespace Engine;

static const auto actionBarSize = 64.0f;

GuiWindowShipToolbar::GuiWindowShipToolbar(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager) :
    GuiWindow{fontFamily, fontSize} {

    setSize({
        actionBarSize * 10.0f + ctx.getPadding().x * 11.0f,
        actionBarSize + ctx.getPadding().y * 2.0f,
    });
    setTitle("Ship Status");
    setCentered(false);
    setBackground(true);
    setNoScrollbar(true);
    setHeader(false);
    setTransparent(true);
    //   setNoInput(true);

    { // Action bar
        auto& row = addWidget<GuiWidgetTemplateRow>(actionBarSize);
        for (auto i = 0; i < 10; i++) {
            auto& item = row.addWidget<GuiWidgetImageToggle>(nullptr);
            item.setWidth(actionBarSize, true);
            item.setLabel(std::to_string((i + 1) % 10));
            item.setTextAlign(GuiTextAlign::LeftBottom);
        }
    }
}

void GuiWindowShipToolbar::updatePos(const Vector2i& viewport) {
    const auto size = getSize();
    setPos({
        static_cast<float>(viewport.x) / 2.0f - size.x / 2.0f,
        static_cast<float>(viewport.y) - size.y,
    });
}
