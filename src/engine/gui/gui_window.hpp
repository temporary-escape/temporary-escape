#pragma once

#include "../graphics/nuklear.hpp"

namespace Engine {
class ENGINE_API GuiWindow : public NuklearWindow {
public:
    GuiWindow();
    virtual ~GuiWindow() = default;

    void draw(Nuklear& nuklear, const Vector2& viewport) override;

    const Vector2& getSize() const;
    const Vector2& getPos() const;
    Nuklear::Flags getFlags() const;
    void setSize(const Vector2& value);
    void setEnabled(bool value);
    void setPos(const Vector2& value);
    void setFlags(unsigned int value);
    const std::string& getTitle() const;
    void setTitle(const std::string& value);
    void setBordered();
    void setAlwaysBackground();
    void setNoScrollbar();
    void setDynamic();
    void setFontSize(int size);
    bool isCursorInside(const Vector2i& mousePos);

    bool isEnabled() const {
        return enabled;
    }

protected:
    virtual void beforeDraw(Nuklear& nuklear, const Vector2& viewport);
    virtual void drawLayout(Nuklear& nuklear) = 0;

    bool enabled{true};
    std::string title;
    Vector2 size{100.0f, 100.0f};
    Vector2 pos{0.0f, 0.0f};
    Nuklear::Flags flags{0};
    int fontSize{0};
};
} // namespace Engine
