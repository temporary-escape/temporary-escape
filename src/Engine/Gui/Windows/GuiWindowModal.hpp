#pragma once

#include "../GuiWindow.hpp"
#include <list>

namespace Engine {
class GuiWindowModal : public GuiWindow {
public:
    using OnChoiceCallback = GuiWidgetButton::OnClickCallback;
    using OnTimeoutCallback = std::function<void()>;

    GuiWindowModal(GuiContext& ctx, const FontFamily& fontFamily, int fontSize, std::string title, std::string text);

    void update(const Vector2i& viewport) override;
    void addChoice(const std::string& label, OnChoiceCallback callback);
    void addChoice(const std::string& label, const GuiStyleButton& style, OnChoiceCallback callback);
    void clearChoices();
    void setTimeout(int value, OnTimeoutCallback callback);
    void setText(std::string value);
    void setCloseOnClick(bool value);

private:
    struct Choice {
        std::string label;
        OnChoiceCallback callback;
        const GuiStyleButton* style;
    };

    void recreateChoices();

    std::string text;
    int timeout{-1};
    GuiWidgetProgressBar* progressBar{nullptr};
    GuiWidgetRow* widgetRowText{nullptr};
    GuiWidgetRow* widgetRowChoices{nullptr};
    GuiWidgetRow* widgetRowTimeout{nullptr};
    GuiWidgetText* widgetText{nullptr};
    std::chrono::steady_clock::time_point start;
    float lastWidth{0.0f};
    OnTimeoutCallback onTimeout;
    bool closeOnClick{true};
    std::list<Choice> choices;
};
} // namespace Engine
