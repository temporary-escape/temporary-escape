#pragma once

#include "../Assets/AssetFontFace.hpp"
#include "../Config.hpp"
#include "../Graphics/Canvas2D.hpp"
#include "../Math/Vector.hpp"
#include "../Platform/Enums.hpp"
#include "GuiFlags.hpp"

#include <functional>
#include <list>
#include <queue>
#include <string>

struct nk_context;
struct nk_user_font;

namespace Scissio {

struct GuiSpace {
    Vector2 pos;
    Vector2 size;
};

struct GuiFontData;

class SCISSIO_API GuiContext {
public:
    explicit GuiContext(Canvas2D& canvas, const Config& config, AssetManager& assetManager);
    virtual ~GuiContext();

    void reset();
    void render(const Vector2& viewport);
    void setFont(const AssetFontFacePtr& fontFace, float height);

    void spacing();
    void window(const Vector2& pos, const Vector2& size, const std::string& name, GuiFlags flags,
                const std::function<void()>& fn);
    void group(const std::string& name, GuiFlags flags, const std::function<void()>& fn);
    bool button(const std::string& text);
    // bool buttonImage(const ImagePtr& image);
    // bool buttonImage(const IconPtr& image, bool active = false);
    void label(const std::string& text);
    void title(const std::string& text);
    void text(const std::string& text);
    void layoutDynamic(float height, int count);
    void combo(const std::string& selected, const Vector2& size, const std::function<void()>& fn);
    bool comboItem(const std::string& text);
    void tooltip(const std::string& text);
    void tooltip(float width, const std::function<void()>& fn);
    bool isNextHover() const;

    Vector2 getContentSize() const;
    Vector2 getMousePos() const;

    void keyPressEvent(const Scissio::Key& key);
    void keyReleaseEvent(const Scissio::Key& key);
    void mousePressEvent(const Vector2i& pos, MouseButton button);
    void mouseReleaseEvent(const Vector2i& pos, MouseButton button);
    void mouseMoveEvent(const Vector2i& pos);
    // void mouseScrollEvent(const Platform::MouseScrollEvent& event);
    void textInputEvent(int c);
    bool inputOverlap(const Vector2i& pos) const;

    const Vector2i& getViewport() const {
        return viewport;
    }

private:
    enum class InputEventType {
        KeyPressed,
        KeyReleased,
        MousePress,
        MouseRelease,
        MouseMove,
        TextInput,
    };

    union InputEventData {
        struct Key {
            Scissio::Key key;
            bool pressed;
        } key;
        struct MousePress {
            MouseButton button;
            int x;
            int y;
        } mousePress;
        struct MouseRelease {
            MouseButton button;
            int x;
            int y;
        } mouseRelease;
        struct MouseMove {
            int x;
            int y;
        } mouseMove;
        struct TextInput {
            int unicode;
        } textInput;
    };

    struct InputEvent {
        InputEventData data;
        InputEventType type;
    };

    struct WindowBounds {
        Vector2i pos;
        Vector2i size;
    };

    void renderInternal(const Vector2& viewport);
    void applyTheme();

    Canvas2D& canvas;
    const Config& config;
    std::unique_ptr<nk_context> ctx;
    Vector2i viewport{};
    std::queue<InputEvent> inputEvents;
    std::unordered_map<AssetFontFacePtr, std::unique_ptr<GuiFontData>> fonts;
    AssetFontFacePtr fontFaceRegular;
    AssetFontFacePtr fontFaceBold;
    std::vector<WindowBounds> windowsBounds;
};
} // namespace Scissio
