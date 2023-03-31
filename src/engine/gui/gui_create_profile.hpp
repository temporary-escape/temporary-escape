#pragma once

#include "gui_window.hpp"

namespace Engine {
class ENGINE_API GuiCreateProfile : public GuiWindow {
public:
    struct Form {
        std::string name;
    };

    explicit GuiCreateProfile();
    ~GuiCreateProfile() override = default;

    template <typename Fn> void setOnSuccess(Fn&& fn) {
        onSuccess = fn;
    }

private:
    void submit();
    void drawLayout(Nuklear& nuklear) override;
    void beforeDraw(Nuklear& nuklear, const Vector2& viewport) override;

    std::function<void(const Form& form)> onSuccess;
    Form form;
};
} // namespace Engine
