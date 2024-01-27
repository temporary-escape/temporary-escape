#include "GuiWindowServerBrowser.hpp"
#include "../GuiManager.hpp"

using namespace Engine;

GuiWindowServerBrowser::GuiWindowServerBrowser(const FontFamily& fontFamily, int fontSize) :
    GuiWindow{fontFamily, fontSize} {

    setSize({600.0f, 700.0f});
    setTitle("Server Browser");
    setNoScrollbar(true);

    const auto height = getSize().y - 30.0 - 30.0f * 2.0f - ctx.getPadding().y * 4.0f;

    { // Top row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);

        auto& label = row.addWidget<GuiWidgetLabel>("Filter:");
        label.setWidth(100.0f, true);

        auto& input = row.addWidget<GuiWidgetTextInput>();
        input.setWidth(0.0f);

        auto& search = row.addWidget<GuiWidgetButton>("Search");
        search.setWidth(100.0f, true);

        auto& reset = row.addWidget<GuiWidgetButton>("Reset");
        reset.setWidth(100.0f, true);
    }

    { // Server list
        auto& row = addWidget<GuiWidgetRow>(height, 1);
        auto& group = row.addWidget<GuiWidgetGroup>();
        group.setScrollbar(true);
        group.setBorder(true);
    }

    { // Bottom row
        auto& row = addWidget<GuiWidgetTemplateRow>(30.0f);
        row.addEmpty().setWidth(0.0f);

        auto& connect = row.addWidget<GuiWidgetButton>("Connect");
        connect.setWidth(100.0f, true);

        buttonClose = &row.addWidget<GuiWidgetButton>("Close");
        buttonClose->setWidth(100.0f, true);
    }
}

void GuiWindowServerBrowser::setOnClickClose(GuiWidgetButton::OnClickCallback callback) {
    buttonClose->setOnClick(std::move(callback));
}
