#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetTextInput : public GuiWidget {
public:
    using OnModifyCallback = std::function<void()>;

    explicit GuiWidgetTextInput(GuiContext& ctx, size_t max = 256);

    void draw() override;

    void setValue(std::string value);
    const std::string& getValue() const {
        return input;
    }

    void setOnModify(OnModifyCallback value);

private:
    const size_t max;
    std::string input;

    OnModifyCallback onModifyCallback;
};
} // namespace Engine
