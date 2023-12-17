#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetCombo : public GuiWidget {
public:
    using OnSelectedCallback = std::function<void(size_t, const std::string&)>;

    GuiWidgetCombo(GuiContext& ctx);

    void draw() override;

    void setOnSelected(OnSelectedCallback value);
    void addChoice(const std::string_view& value);
    void setChosen(size_t value);

private:
    OnSelectedCallback onClick;
    std::vector<std::string> choices;
    size_t chosen{0};
};
} // namespace Engine
