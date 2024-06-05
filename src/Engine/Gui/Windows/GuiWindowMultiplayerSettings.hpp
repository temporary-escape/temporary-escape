#pragma once

#include "../GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiWindowMultiplayerSettings : public GuiWindow {
public:
    struct Form {
        std::string name;
        std::string password;
    };

    using OnStartCallback = std::function<void(const Form&)>;

    explicit GuiWindowMultiplayerSettings(GuiContext& ctx, const FontFamily& fontFamily, int fontSize);

    void setOnStart(OnStartCallback callback);
    void setServerName(std::string value);

private:
    void validate();

    OnStartCallback onStart;
    GuiWidgetTextInput* inputName;
    GuiWidgetTextInput* inputPassword;
    GuiWidgetLabel* labelError;
    bool valid{false};
};
} // namespace Engine
