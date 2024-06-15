#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowCreateProfile : public GuiWindow {
public:
    struct Result {
        std::string name;
    };

    using OnCreateCallback = std::function<void(const Result&)>;

    GuiWindowCreateProfile(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);

    void setOnCreateCallback(OnCreateCallback value);

private:
    OnCreateCallback onCreateCallback;
    GuiWidgetTextInput* input;
    GuiWidgetLabel* labelError;
    GuiWidgetButton* button;
    bool valid{true};
};
} // namespace Engine
