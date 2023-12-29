#include "GuiWindowShipToolbar.hpp"
#include "../../Assets/AssetsManager.hpp"

using namespace Engine;

static const auto colorShield = hexColorGamma(0x339bd8ff);
static const auto colorHealth = hexColorGamma(0xa4d949ff);
static const auto colorBattery = hexColorGamma(0xfed839ff);
static const auto colorGenerated = hexColorGamma(0xfed839ff);
static const auto progressHeight = 16.0f;
static const auto actionBarSize = 64.0f;

const GuiStyleProgress GuiWindowShipToolbar::styleProgressShields = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorShield,
            .hover = colorShield,
            .active = colorShield,
        },
};

const GuiStyleProgress GuiWindowShipToolbar::styleProgressHealth = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorHealth,
            .hover = colorHealth,
            .active = colorHealth,
        },
};

const GuiStyleProgress GuiWindowShipToolbar::styleProgressBattery = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorBattery,
            .hover = colorBattery,
            .active = colorBattery,
        },
};

const GuiStyleProgress GuiWindowShipToolbar::styleProgressGenerated = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorGenerated,
            .hover = colorGenerated,
            .active = colorGenerated,
        },
};

GuiWindowShipToolbar::GuiWindowShipToolbar(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager) :
    GuiWindow{fontFamily, fontSize} {

    setSize({
        actionBarSize * 10.0f + ctx.getPadding().x * 11.0f,
        actionBarSize + progressHeight * 2.0f + ctx.getPadding().y * 4.0f,
    });
    setTitle("Ship Status");
    setBackground(true);
    setNoScrollbar(true);
    setHeader(false);
    setTransparent(true);
    //   setNoInput(true);

    const auto totalWidth = getSize().x - ctx.getPadding().x * 2.0f;
    const auto halfWidth = totalWidth / 2.0f - ctx.getPadding().x / 2.0f;
    const auto progressWidth = halfWidth - progressHeight - ctx.getPadding().x;

    { // First row
        auto& row = addWidget<GuiWidgetTemplateRow>(progressHeight);

        { // Shields
            auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_shield"));
            image.setColor(colorShield);
            image.setWidth(progressHeight, true);

            progressShields = &row.addWidget<GuiWidgetProgressBar>();
            progressShields->setValue(1000.0f);
            progressShields->setMax(1000.0f);
            progressShields->setHeight(progressHeight / 2.0f);
            progressShields->setStyle(&styleProgressShields);
            progressShields->setWidth(progressWidth, true);
            progressShields->setTooltip("Shields");
        }

        { // Health
            progressHull = &row.addWidget<GuiWidgetProgressBar>();
            progressHull->setValue(1000.0f);
            progressHull->setMax(1000.0f);
            progressHull->setHeight(progressHeight / 2.0f);
            progressHull->setStyle(&styleProgressHealth);
            progressHull->setWidth(progressWidth, true);
            progressHull->setTooltip("Health");

            auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_health"));
            image.setColor(colorHealth);
            image.setWidth(progressHeight, true);
        }
    }

    { // Second row
        auto& row = addWidget<GuiWidgetTemplateRow>(progressHeight);

        { // Battery energy
            auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_battery_pack"));
            image.setColor(colorBattery);
            image.setWidth(progressHeight, true);

            progressBattery = &row.addWidget<GuiWidgetProgressBar>();
            progressBattery->setValue(1000.0f);
            progressBattery->setMax(1000.0f);
            progressBattery->setHeight(progressHeight / 2.0f);
            progressBattery->setStyle(&styleProgressBattery);
            progressBattery->setWidth(progressWidth, true);
            progressBattery->setTooltip("Battery Energy");
        }

        { // Generated energy
            progressGenerated = &row.addWidget<GuiWidgetProgressBar>();
            progressGenerated->setValue(1000.0f);
            progressGenerated->setMax(1000.0f);
            progressGenerated->setHeight(progressHeight / 2.0f);
            progressGenerated->setStyle(&styleProgressGenerated);
            progressGenerated->setWidth(progressWidth, true);
            progressGenerated->setTooltip("Generated Energy");

            auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_power_lightning"));
            image.setColor(colorGenerated);
            image.setWidth(progressHeight, true);
        }
    }

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
