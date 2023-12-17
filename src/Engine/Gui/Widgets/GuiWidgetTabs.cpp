#include "GuiWidgetTabs.hpp"
#include "GuiWidgetButtonToggle.hpp"

using namespace Engine;

GuiWidgetTabs::GuiWidgetTabs(GuiContext& ctx, const float height) :
    GuiWidget{ctx}, height{height}, rowButtons{ctx, 30.0f, 0} {
}

void GuiWidgetTabs::draw() {
    if (groups.empty()) {
        return;
    }

    rowButtons.draw();

    ctx.layoutRowBegin(height - 30.0f, 1);
    ctx.layoutRowPush(1.0f);

    size_t counter{0};
    for (auto& group : groups) {
        if (current == counter++) {
            group.draw();
            break;
        }
    }

    ctx.layoutRowEnd();
}

GuiWidgetGroup& GuiWidgetTabs::addTab(const std::string& label) {
    // Add a new toggle button
    auto& toggle = rowButtons.addWidget<GuiWidgetButtonToggle>(label);
    toggles.push_back(&toggle);
    auto index = static_cast<int>(toggles.size() - 1);

    if (index == 0) {
        toggle.setValue(true);
    }

    rowButtons.setColumns(static_cast<int>(toggles.size()));

    for (auto* t : toggles) {
        t->setWidth(1.0f / static_cast<float>(toggles.size()));
    }

    toggle.setOnClick([this, index](bool value) {
        // Remember the current index selected
        this->current = index;

        // Un-toggle other buttons
        for (size_t i = 0; i < this->toggles.size(); i++) {
            this->toggles[i]->setValue(i == this->current);
        }
    });

    groups.emplace_back(ctx);
    return groups.back();
}
