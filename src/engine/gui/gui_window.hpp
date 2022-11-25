#pragma once

#include "../graphics/nuklear.hpp"

namespace Engine {
class ENGINE_API GuiWindow {
public:
    explicit GuiWindow(Nuklear& nuklear);

    virtual ~GuiWindow() = default;

    void draw(const Vector2i& viewport);

    const Vector2& getSize() const;
    const Vector2& getPos() const;
    Nuklear::Flags getFlags() const;
    void setSize(const Vector2& size);
    void setPos(const Vector2& pos);
    void setFlags(unsigned int flags);
    const std::string& getTitle() const;
    void setTitle(const std::string& title);
    void setBordered();
    void setWithBackground();
    void setNoScrollbar();
    void setDynamic();

protected:
    virtual void beforeDraw(const Vector2i& viewport);
    virtual void drawLayout() = 0;

    Nuklear& nuklear;
    std::string title;
    Vector2 size{100.0f, 100.0f};
    Vector2 pos{0.0f, 0.0f};
    Nuklear::Flags flags{0};
};
} // namespace Engine
