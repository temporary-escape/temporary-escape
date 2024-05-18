#include "GuiWindowShipStatus.hpp"
#include "../../Assets/AssetsManager.hpp"

using namespace Engine;

static const auto colorShield = hexColorGamma(0x339bd8ff);
static const auto colorHealth = hexColorGamma(0xa4d949ff);
static const auto colorEnergy = hexColorGamma(0xfed839ff);
static const auto progressHeight = 16.0f;

const GuiStyleProgress GuiWindowShipStatus::styleProgressShields = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorShield,
            .hover = colorShield,
            .active = colorShield,
        },
};

const GuiStyleProgress GuiWindowShipStatus::styleProgressHealth = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorHealth,
            .hover = colorHealth,
            .active = colorHealth,
        },
};

const GuiStyleProgress GuiWindowShipStatus::styleProgressEnergy = {
    .border = GuiWidgetProgressBar::defaultStyle.border,
    .bar =
        {
            .normal = colorEnergy,
            .hover = colorEnergy,
            .active = colorEnergy,
        },
};

GuiWindowShipStatus::GuiWindowShipStatus(GuiContext& ctx, const FontFamily& fontFamily, int fontSize,
                                         AssetsManager& assetsManager) :
    GuiWindow{ctx, fontFamily, fontSize} {

    setSize({
        350.0f,
        progressHeight * 3.0f + ctx.getPadding().x * 6.0f,
    });
    setTitle("Ship Status");
    setCentered(false);
    setBackground(true);
    setNoScrollbar(true);
    setHeader(false);
    setOpacity(0.0f);
    setBordered(false);
    //   setNoInput(true);

    const auto totalWidth = getSize().x - ctx.getPadding().x * 2.0f;
    const auto totalHeight = getSize().y - ctx.getPadding().y * 2.0f;
    const auto groupWidth = totalWidth - totalHeight - ctx.getPadding().x * 1.0f;
    const auto progressWidth = groupWidth - progressHeight - ctx.getPadding().x * 2.0f;

    auto& main = addWidget<GuiWidgetTemplateRow>(totalHeight);

    { // Ship image
        auto& image = main.addWidget<GuiWidgetImage>(assetsManager.getImages().find("image_ship_hud"));
        image.setColor(Colors::text);
        image.setWidth(totalHeight, true);
    }

    auto& group = main.addWidget<GuiWidgetGroup>();
    group.setScrollbar(false);
    group.setBorder(false);
    group.setWidth(groupWidth, true);

    { // Shields
        auto& row = group.addWidget<GuiWidgetTemplateRow>(progressHeight);

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
        auto& row = group.addWidget<GuiWidgetTemplateRow>(progressHeight);

        auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_health"));
        image.setColor(colorHealth);
        image.setWidth(progressHeight, true);

        progressHealth = &row.addWidget<GuiWidgetProgressBar>();
        progressHealth->setValue(1000.0f);
        progressHealth->setMax(1000.0f);
        progressHealth->setHeight(progressHeight / 2.0f);
        progressHealth->setStyle(&styleProgressHealth);
        progressHealth->setWidth(progressWidth, true);
        progressHealth->setTooltip("Health");
    }

    { // Energy
        auto& row = group.addWidget<GuiWidgetTemplateRow>(progressHeight);

        auto& image = row.addWidget<GuiWidgetImage>(assetsManager.getImages().find("icon_battery_pack"));
        image.setColor(colorEnergy);
        image.setWidth(progressHeight, true);

        progressEnergy = &row.addWidget<GuiWidgetProgressBar>();
        progressEnergy->setValue(1000.0f);
        progressEnergy->setMax(1000.0f);
        progressEnergy->setHeight(progressHeight / 2.0f);
        progressEnergy->setStyle(&styleProgressEnergy);
        progressEnergy->setWidth(progressWidth, true);
        progressEnergy->setTooltip("Energy");
    }
}

void GuiWindowShipStatus::update(const Vector2i& viewport) {
    GuiWindow::update(viewport);
    
    const auto size = getSize();
    setPos({
        25.0f,
        25.0f,
    });
}
