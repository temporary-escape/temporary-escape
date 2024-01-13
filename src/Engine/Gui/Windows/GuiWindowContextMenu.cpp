#include "GuiWindowContextMenu.hpp"

using namespace Engine;

static int menuIndex{0};

GuiWindowContextMenu::GuiWindowContextMenu(const FontFamily& fontFamily, int fontSize, GuiWindowContextMenu* child) :
    GuiWindow{fontFamily, fontSize}, child{child} {
    setSize({200.0f, 600.0f});
    setCentered(false);
    setTitle(fmt::format("Context Menu {}", menuIndex++));
    setPadding(0.0f);
    setDynamic(true);
    setHeader(false);
    setNoScrollbar(true);

    row = &addWidget<GuiWidgetRow>(static_cast<float>(fontSize) * 1.25f, 1);
}

void GuiWindowContextMenu::update(const Vector2i& viewport) {
    if (child && !isEnabled() && child->isEnabled()) {
        child->setEnabled(false);
    }
    GuiWindow::update(viewport);
}

GuiWidgetContextButton& GuiWindowContextMenu::addItem(ImagePtr image, std::string label,
                                                      GuiWidgetContextButton::OnClickCallback callback) {
    auto& button = row->addWidget<GuiWidgetContextButton>(std::move(image), nullptr, std::move(label));
    button.setOnClick(std::move(callback));
    button.setOnHover([this](const bool blur) {
        if (child && blur) {
            child->setEnabled(false);
        }
    });
    return button;
}

void GuiWindowContextMenu::addNested(ImagePtr imageLeft, ImagePtr imageRight, std::string label,
                                     NestedCallback callback) {
    if (!child) {
        EXCEPTION("Gui context menu has no child");
    }

    const auto xOffset = static_cast<float>(row->getCount()) * (static_cast<float>(getFontSize()) * 1.25f + 4.0f);

    auto& button =
        row->addWidget<GuiWidgetContextButton>(std::move(imageLeft), std::move(imageRight), std::move(label));

    button.setOnHover([this, xOffset, c = std::move(callback)](const bool blur) {
        if (blur) {
            child->clear();

            c(*child);

            const Vector2 offset{
                getPos().x + getSize().x,
                getPos().y + xOffset,
            };

            child->setEnabled(blur);
            child->setPos(offset);
        }
    });
}

void GuiWindowContextMenu::clear() {
    row->clearWidgets();
    if (child) {
        child->clear();
    }
}
