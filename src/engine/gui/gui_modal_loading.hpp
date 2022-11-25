#pragma once

#include "../assets/block.hpp"
#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiModalLoading : public GuiWindow {
public:
    explicit GuiModalLoading(Nuklear& nuklear, const std::string& title);

    ~GuiModalLoading() override = default;

    void setProgress(const float value) {
        progress = value;
    }

private:
    void drawLayout() override;
    void beforeDraw(const Vector2i& viewport) override;

    float progress;
};
} // namespace Engine
