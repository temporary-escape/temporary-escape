#pragma once

#include "../font/font_family.hpp"
#include "../window.hpp"
#include "canvas.hpp"
#include <functional>
#include <list>

struct nk_context;
struct nk_user_font;

namespace Engine {
class ENGINE_API NuklearWindow;

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

    enum class TextAlign : uint32_t {
        Left = 0x01 | 0x10,
        Center = 0x02 | 0x10,
        Right = 0x04 | 0x10,
        LeftTop = 0x01 | 0x08,
        CenterTop = 0x02 | 0x08,
        RightTop = 0x04 | 0x08,
        LeftBottom = 0x01 | 0x20,
        CenterBottom = 0x02 | 0x20,
        RightBottom = 0x04 | 0x20,
    };

    explicit Nuklear(Canvas& canvas, const FontFace& fontFace, float fontSize);
    ~Nuklear();

    void begin(const Vector2i& viewport);
    void draw(NuklearWindow& window);
    void end();

    bool beginWindow(const std::string& title, const Vector2& pos, const Vector2& size, Flags flags);
    void endWindow();

    void layoutDynamic(float height, int count);
    bool button(const std::string& text, TextAlign align = TextAlign::Center);
    void label(const std::string& text);
    void progress(float value);

    void eventMouseMoved(const Vector2i& pos);
    void eventMousePressed(const Vector2i& pos, MouseButton button);
    void eventMouseReleased(const Vector2i& pos, MouseButton button);
    void eventMouseScroll(int xscroll, int yscroll);
    void eventKeyPressed(Key key, Modifiers modifiers);
    void eventKeyReleased(Key key, Modifiers modifiers);
    void eventCharTyped(uint32_t code);

    [[nodiscard]] const Vector2i& getViewport() const {
        return lastViewportValue;
    }

private:
    void applyTheme();
    void input();
    void render();

    static inline const auto padding = 4.0f;

    Canvas& canvas;
    Vector2i lastViewportValue;
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

class ENGINE_API NuklearWindow {
public:
    virtual ~NuklearWindow() = default;

    virtual void draw(Nuklear& nuklear, const Vector2& viewport) = 0;
};
} // namespace Engine
