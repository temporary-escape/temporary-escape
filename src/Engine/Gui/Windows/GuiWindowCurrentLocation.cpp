#include "GuiWindowCurrentLocation.hpp"
#include "../../Assets/AssetsManager.hpp"

using namespace Engine;

GuiWindowCurrentLocation::GuiWindowCurrentLocation(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                                   AssetsManager& assetsManager) :
    GuiWindow{ctx, fontFamily, fontSize} {

    setSize({
        350.0f,
        100.0f,
    });
    setTitle("Current Location");
    setCentered(false);
    setBackground(true);
    setNoScrollbar(true);
    setHeader(false);
    setOpacity(0.0f);
    setBordered(false);
    //   setNoInput(true);

    const auto totalWidth = getSize().x - ctx.getPadding().x * 2.0f;
    const auto totalHeight = getSize().y - ctx.getPadding().y * 2.0f;
    const auto imageSize = fontSize;
    const auto labelWidth = totalWidth - imageSize - ctx.getPadding().x;

    {
        auto& row = addWidget<GuiWidgetTemplateRow>(fontSize);

        auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_position_marker"));
        image.setWidth(imageSize, true);
        image.setColor(Colors::text);

        auto& label = row.addWidget<GuiWidgetLabel>("Unknown system");
        label.setWidth(labelWidth, true);
        systemLabel = &label;
    }

    {
        auto& row = addWidget<GuiWidgetTemplateRow>(fontSize);
        row.addEmpty().setWidth(imageSize, true);

        auto& label = row.addWidget<GuiWidgetLabel>("Unknown sector");
        label.setWidth(labelWidth, true);
        sectorLabel = &label;
    }
}

void GuiWindowCurrentLocation::setSystemLabel(const std::string& value) {
    systemLabel->setLabel(value);
}

void GuiWindowCurrentLocation::setSectorLabel(const std::string& value) {
    sectorLabel->setLabel(value);
}

void GuiWindowCurrentLocation::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);
    
    const auto size = getSize();
    setPos({
        25.0f,
        125.0f,
    });
}
