#pragma once

#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiModalLoading : public GuiWindow {
public:
    explicit GuiModalLoading(const std::string& title);
    ~GuiModalLoading() override = default;

    void setProgress(const float value) {
        progress = value;
    }

private:
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    float progress;
};
} // namespace Engine
