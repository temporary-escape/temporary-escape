#pragma once

#include "../GuiWidget.hpp"

namespace Engine {
class ENGINE_API GuiWidgetLayout : public GuiWidget {
public:
    explicit GuiWidgetLayout(GuiContext& ctx);

    void draw() override;

    template <typename T, typename... Args> T& addWidget(Args&&... args) {
        auto* ptr = new T(ctx, std::forward<Args>(args)...);
        widgets.push_back(std::shared_ptr<GuiWidget>(static_cast<GuiWidget*>(ptr)));
        return *ptr;
    }
    GuiWidget& addEmpty() {
        return addWidget<GuiWidget>();
    }
    void clearWidgets();
    void removeWidget(GuiWidget& widget);

protected:
    std::vector<std::shared_ptr<GuiWidget>> widgets;
};
} // namespace Engine
