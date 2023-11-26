#pragma once

#include "GuiWindow.hpp"

namespace Engine {
class ENGINE_API GuiKeepSettings : public GuiWindow {
public:
    explicit GuiKeepSettings();
    ~GuiKeepSettings() override = default;

    void reset();
    void updateProgress(float deltaTime);
    template <typename Fn> void setOnResult(Fn&& fn) {
        onResult = std::forward<Fn>(fn);
    }

private:
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    float progress{1.0f};
    float timeout{10.0f};
    std::function<void(bool)> onResult;
};
} // namespace Engine
