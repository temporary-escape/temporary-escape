#pragma once

#include "../Font/FontFamily.hpp"
#include "../Vulkan/Window.hpp"
#include "Canvas.hpp"
#include <functional>
#include <list>

struct nk_context;
struct nk_user_font;

namespace Engine {
class ENGINE_API Nuklear {
public:
    using Flags = uint32_t;

    enum class WindowFlags {
        Border = (1 << (0)),
        Moveable = (1 << (1)),
        Scaleable = (1 << (2)),
        Closeable = (1 << (3)),
        Minimizable = (1 << (4)),
        NoScrollbar = (1 << (5)),
        Title = (1 << (6)),
        ScrollAutoHide = (1 << (7)),
        Background = (1 << (8)),
        ScaleLeft = (1 << (9)),
        NoInput = (1 << (10)),
        Dynamic = (1 << (11)),
        CenterX = (1 << (29)),
        CenterY = 1 << 30,
    };

    explicit Nuklear(Canvas& canvas, const FontFace& fontFace, float fontSize);
    ~Nuklear();

    void begin(const Vector2i& viewport);
    void end();

    bool beginWindow(const std::string& title, const Vector2& pos, const Vector2& size, Flags flags);
    void endWindow();

    void layoutDynamic(float height, int count);
    bool button(const std::string& text);

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);

private:
    void applyTheme();
    void input();
    void render();

    static inline const auto padding = 4.0f;

    Canvas& canvas;
    std::unique_ptr<nk_context> ctx;
    std::unique_ptr<nk_user_font> ctxFont;
    std::vector<std::tuple<Vector2, Vector2>> windowsBounds;
    std::list<std::function<void()>> inputEvents;
};

inline Nuklear::Flags operator|(const Nuklear::WindowFlags a, const Nuklear::WindowFlags b) {
    return static_cast<Nuklear::Flags>(a) | static_cast<Nuklear::Flags>(b);
}

inline Nuklear::Flags operator|(const Nuklear::Flags a, const Nuklear::WindowFlags b) {
    return a | static_cast<Nuklear::Flags>(b);
}
} // namespace Engine
