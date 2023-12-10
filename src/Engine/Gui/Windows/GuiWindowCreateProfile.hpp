#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class GuiWindowCreateProfile : public GuiWindow2 {
public:
    struct Result {
        std::string name;
    };

    using OnCreateCallback = std::function<void(const Result&)>;

    GuiWindowCreateProfile(const FontFamily& fontFamily, int fontSize);

    void setOnCreateCallback(OnCreateCallback value);

private:
    OnCreateCallback onCreateCallback;
    GuiWidgetTextInput* input;
    GuiWidgetLabel* labelError;
    bool valid{true};
};
} // namespace Engine
