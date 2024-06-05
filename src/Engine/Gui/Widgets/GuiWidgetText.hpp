#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetText : public GuiWidget {
public:
    explicit GuiWidgetText(GuiContext& ctx, const std::string& text, float maxWidth);

    void setText(const std::string& value, float maxWidth);
    float getSuggestedHeight() const {
        return suggestedHeight;
    }

private:
    void drawInternal() override;

    std::string text;
    float suggestedHeight{0.0f};
};
} // namespace Engine
