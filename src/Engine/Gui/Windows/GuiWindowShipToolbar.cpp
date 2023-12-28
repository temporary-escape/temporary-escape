#include "GuiWindowShipToolbar.hpp"
#include "../../Assets/AssetsManager.hpp"

using namespace Engine;

static const auto colorShield = hexColorGamma(0x339bd8ff);
static const auto colorHealth = hexColorGamma(0xa4d949ff);
static const auto colorEnergy = hexColorGamma(0xfed839ff);
static const auto colorHyperspace = hexColorGamma(0xfed839ff);
static const auto progressHeight = 16.0f;
static const auto actionBarSize = 64.0f;

GuiWindowShipToolbar::GuiWindowShipToolbar(const FontFamily& fontFamily, int fontSize, AssetsManager& assetsManager) :
    GuiWindow2{fontFamily, fontSize} {

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
            progressShields->setColor(colorShield);
            progressShields->setWidth(progressWidth, true);
            progressShields->setTooltip("Shields");
        }

        { // Health
            progressHull = &row.addWidget<GuiWidgetProgressBar>();
            progressHull->setValue(1000.0f);
            progressHull->setMax(1000.0f);
            progressHull->setColor(colorHealth);
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
            image.setColor(colorEnergy);
            image.setWidth(progressHeight, true);

            progressBattery = &row.addWidget<GuiWidgetProgressBar>();
            progressBattery->setValue(1000.0f);
            progressBattery->setMax(1000.0f);
            progressBattery->setColor(colorEnergy);
            progressBattery->setWidth(progressWidth, true);
            progressBattery->setTooltip("Battery Energy");
        }

        { // Generated energy
            progressGenerated = &row.addWidget<GuiWidgetProgressBar>();
            progressGenerated->setValue(1000.0f);
            progressGenerated->setMax(1000.0f);
            progressGenerated->setColor(colorHyperspace);
            progressGenerated->setWidth(progressWidth, true);
            progressGenerated->setTooltip("Generated Energy");

            auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_power_lightning"));
            image.setColor(colorHyperspace);
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
