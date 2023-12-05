#include "GuiContext.hpp"
#include "../Graphics/Theme.hpp"
#define NK_IMPLEMENTATION 1
#define NK_INCLUDE_DEFAULT_ALLOCATOR 1
#include <nuklear.h>

using namespace Engine;

static Color4 asColor(const nk_color& color) {
    return {static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f,
            static_cast<float>(color.a) / 255.0f};
}

static nk_color HEX(const uint32_t v) {
    return nk_rgba((v & 0xFF000000) >> 24, (v & 0x00FF0000) >> 16, (v & 0x0000FF00) >> 8, (v & 0x000000FF) >> 0);
}

static nk_color fromColor(const Color4& color) {
    return nk_rgba(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

static float getTextWidth(const nk_handle handle, const float height, const char* str, const int len) {
    auto& font = *static_cast<const FontFace*>(handle.ptr);
    return font.getBounds({str, static_cast<size_t>(len)}, height).x;
}

static nk_buttons toNkButtons(const MouseButton button) {
    switch (button) {
    case MouseButton::Left: {
        return nk_buttons::NK_BUTTON_LEFT;
    }
    case MouseButton::Right: {
        return nk_buttons::NK_BUTTON_RIGHT;
    }
    case MouseButton::Middle: {
        return nk_buttons::NK_BUTTON_MIDDLE;
    }
    default: {
        return nk_buttons::NK_BUTTON_MAX;
    }
    }
}

static nk_keys toNkKeys(const Key key) {
    switch (key) {
    case Key::LeftShift: {
        return nk_keys::NK_KEY_SHIFT;
    }
    case Key::RightShift: {
        return nk_keys::NK_KEY_SHIFT;
    }
    case Key::Backspace: {
        return nk_keys::NK_KEY_BACKSPACE;
    }
    case Key::LeftControl: {
        return nk_keys::NK_KEY_CTRL;
    }
    case Key::RightControl: {
        return nk_keys::NK_KEY_CTRL;
    }
    case Key::Delete: {
        return nk_keys::NK_KEY_DEL;
    }
    case Key::Tab: {
        return nk_keys::NK_KEY_TAB;
    }
    default: {
        return nk_keys::NK_KEY_NONE;
    }
    }
}

GuiContext::GuiContext(const FontFamily& fontFamily, const int fontSize) :
    fontFamily{fontFamily}, fontSize{fontSize}, nk{std::make_unique<nk_context>()} {

    for (size_t i = 0; i < FontFamily::total; i++) {
        auto& font = fonts.at(i);
        auto& face = fontFamily.get(static_cast<FontFace::Type>(i));
        font = std::make_unique<nk_user_font>();

        font->height = static_cast<float>(fontSize);
        font->width = &getTextWidth;
        font->userdata.ptr = const_cast<void*>(reinterpret_cast<const void*>(&face));
    }

    nk_init_default(nk.get(), fonts.at(0).get());
    applyTheme();
}

GuiContext::~GuiContext() {
    if (nk) {
        nk_free(nk.get());
    }
}

void GuiContext::update() {
    if (!inputEvents.empty()) {
        nk_input_begin(nk.get());
        for (const auto& event : inputEvents) {
            event();
        }
        inputEvents.clear();
        nk_input_end(nk.get());
    }
}

void GuiContext::render(Canvas2& canvas) {
    const struct nk_command* cmd;
    nk_foreach(cmd, nk.get()) {
        switch (cmd->type) {
        case NK_COMMAND_SCISSOR: {
            const auto c = reinterpret_cast<const struct nk_command_scissor*>(cmd);
            if (c->x >= 0 && c->y >= 0) {
                canvas.setScissor({static_cast<float>(c->x), static_cast<float>(c->y)},
                                  {static_cast<float>(c->w), static_cast<float>(c->h)});
            } else {
                canvas.clearScissor();
            }
            break;
        }
        case NK_COMMAND_LINE: {
            const auto c = reinterpret_cast<const struct nk_command_line*>(cmd);
            /*if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.moveTo(Vector2(c->begin.x, c->begin.y));
            canvas.strokeColor(asColor(c->color));
            canvas.strokeWidth(c->line_thickness);
            canvas.lineTo(Vector2(c->end.x, c->end.y));
            canvas.stroke();
            canvas.closePath();*/
            break;
        }
        case NK_COMMAND_TEXT: {
            const auto c = reinterpret_cast<const struct nk_command_text*>(cmd);
            const auto& font = *static_cast<const FontFamily*>(c->font->userdata.ptr);
            std::string_view text{&c->string[0], std::strlen(&c->string[0])};
            const auto pos = Vector2{c->x, static_cast<float>(c->y) + c->height / 1.25f};
            canvas.drawText(pos, text, font, c->font->height, asColor(c->foreground));
            break;
        }
        case NK_COMMAND_RECT: {
            const auto c = reinterpret_cast<const struct nk_command_rect*>(cmd);
            const auto p = Vector2{c->x, c->y};
            const auto s = Vector2{c->w, c->h};
            canvas.drawRectOutline(p, s, c->line_thickness, asColor(c->color));
            break;
        }
        case NK_COMMAND_RECT_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_rect_filled*>(cmd);
            canvas.drawRect(Vector2(c->x, c->y), Vector2(c->w, c->h), asColor(c->color));
            break;
        }
        case NK_COMMAND_TRIANGLE: {
            const auto c = reinterpret_cast<const struct nk_command_triangle*>(cmd);
            /*if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.strokeColor(asColor(c->color));
            canvas.strokeWidth(c->line_thickness);
            canvas.moveTo(asVec(c->a));
            canvas.lineTo(asVec(c->b));
            canvas.lineTo(asVec(c->c));
            canvas.lineTo(asVec(c->a));
            canvas.stroke();*/
            break;
        }
        case NK_COMMAND_TRIANGLE_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_triangle_filled*>(cmd);
            /*if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.fillColor(asColor(c->color));
            canvas.moveTo(asVec(c->a));
            canvas.lineTo(asVec(c->b));
            canvas.lineTo(asVec(c->c));
            canvas.lineTo(asVec(c->a));
            canvas.fill();*/
            break;
        }
        case NK_COMMAND_IMAGE: {
            /*const auto c = reinterpret_cast<const struct nk_command_image*>(cmd);
            const auto& color = asColor(c->col);
            const auto image = reinterpret_cast<Image*>(c->img.handle.ptr);
            if (image) {
                canvas.color(color);
                canvas.image({static_cast<float>(c->x), static_cast<float>(c->y)},
                             {static_cast<float>(c->w), static_cast<float>(c->h)},
                             *image);
            }*/
            break;
        }
        default: {
            break;
        }
        }
    }
    nk_clear(nk.get());
}

bool GuiContext::beginWindow(const std::string& title, const Vector2& pos, const Vector2& size,
                             GuiContext::Flags flags) {
    if (flags & WindowFlag::Transparent) {
        nk_style_push_color(nk.get(), &nk->style.window.background, nk_rgba(0, 0, 0, 0));
        nk_style_push_style_item(
            nk.get(), &nk->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
        resetStyle = true;
    }

    if (nk_begin_titled(nk.get(), title.c_str(), title.c_str(), nk_rect(pos.x, pos.y, size.x, size.y), flags)) {
        // windowsBounds.emplace_back(pos, size);
        /*if (!(flags & static_cast<Flags>(WindowFlag::Moveable))) {
            nk_window_set_position(nk.get(), title.c_str(), nk_vec2(pos.x, pos.y));
        }*/

        /*nk_layout_row_dynamic(nk.get(), 30, 2);

        nk_button_label(nk.get(), "Hello");
        nk_button_label(nk.get(), "World");
        nk_button_label(nk.get(), "First");
        nk_button_label(nk.get(), "Second");*/

        return true;
    }

    return false;
}

void GuiContext::endWindow() {
    nk_end(nk.get());

    if (resetStyle) {
        resetStyle = false;
        nk_style_pop_color(nk.get());
        nk_style_pop_style_item(nk.get());
    }
}

void GuiContext::layoutRowBegin(const float height, const int columns) {
    nk_layout_row_begin(nk.get(), NK_DYNAMIC, height, columns);
}

void GuiContext::layoutRowPush(const float width) {
    nk_layout_row_push(nk.get(), width);
}

void GuiContext::layoutRowEnd() {
    nk_layout_row_end(nk.get());
}

bool GuiContext::button(const std::string& label) {
    if (nk_button_label(nk.get(), label.c_str())) {
        return true;
    }
    return false;
}

void GuiContext::eventMouseMoved(const Vector2i& pos) {
    inputEvents.emplace_back([=]() { nk_input_motion(nk.get(), pos.x, pos.y); });
    setDirty();
}

void GuiContext::eventMousePressed(const Vector2i& pos, MouseButton button) {
    inputEvents.emplace_back([=]() { nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, true); });
    setDirty();
}

void GuiContext::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    inputEvents.emplace_back([=]() { nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, false); });
    setDirty();
}

void GuiContext::eventMouseScroll(int xscroll, int yscroll) {
}

void GuiContext::eventKeyPressed(Key key, Modifiers modifiers) {
}

void GuiContext::eventKeyReleased(Key key, Modifiers modifiers) {
}

void GuiContext::eventCharTyped(uint32_t code) {
}

static const auto BLACK = HEX(0x000000ff);
static const auto PRIMARY_COLOR = fromColor(Theme::primary);
static const auto WHITE = HEX(0xe5e5e3ff);
static const auto TEXT_WHITE = fromColor(Theme::text);
static const auto TEXT_BLACK = HEX(0x030303ff);
static const auto TEXT_GREY = HEX(0x393939ff);
static const auto BACKGROUND_COLOR = fromColor(Theme::backgroundTransparent); // HEX(0x0a0a0aff);
static const auto BORDER_GREY = HEX(0x202020ff);
static const auto TRANSPARENT_COLOR = HEX(0x00000000);
static const auto ACTIVE_COLOR = PRIMARY_COLOR;

void GuiContext::applyTheme() {
    auto& window = nk->style.window;
    auto& button = nk->style.button;
    auto& checkbox = nk->style.checkbox;
    auto& selectable = nk->style.selectable;
    auto& text = nk->style.text;
    auto& combo = nk->style.combo;
    auto& contextual_button = nk->style.contextual_button;
    auto& progress = nk->style.progress;

    text.color = TEXT_WHITE;
    text.padding.x = 0;
    text.padding.y = 2;

    // window.background = PRIMARY_COLOR;
    window.border = 1.0f;
    window.border_color = BORDER_GREY;
    window.rounding = 0;
    window.fixed_background.data.color = BACKGROUND_COLOR;
    window.header.normal.data.color = ACTIVE_COLOR;
    window.header.hover.data.color = ACTIVE_COLOR;
    window.header.active.data.color = ACTIVE_COLOR;
    window.header.close_button.normal.data.color = BACKGROUND_COLOR;
    window.header.close_button.hover.data.color = WHITE;
    window.header.close_button.active.data.color = ACTIVE_COLOR;
    window.header.close_button.text_normal = TEXT_WHITE;
    window.header.close_button.text_hover = TEXT_BLACK;
    window.header.close_button.text_active = TEXT_BLACK;
    window.header.label_normal = TEXT_BLACK;
    window.header.label_hover = TEXT_BLACK;
    window.header.label_active = TEXT_BLACK;
    window.header.padding = nk_vec2(2, 1);
    window.group_padding = nk_vec2(0, 0);
    window.padding = nk_vec2(padding, padding);
    window.group_border = 1.0f;
    window.background = BACKGROUND_COLOR;
    window.min_row_height_padding = 0;
    window.combo_border_color = ACTIVE_COLOR;
    window.combo_border = 1.0f;

    combo.border = 1.0f;
    combo.border_color = BORDER_GREY;
    combo.rounding = 0;
    combo.normal.data.color = BACKGROUND_COLOR;
    combo.hover.data.color = WHITE;
    combo.active.data.color = ACTIVE_COLOR;
    // combo.button_padding = nk_vec2(0, 0);
    // combo.content_padding = nk_vec2(0, 0);
    combo.label_normal = TEXT_WHITE;
    combo.label_hover = TEXT_BLACK;
    combo.label_active = TEXT_BLACK;
    combo.symbol_normal = TEXT_WHITE;
    combo.symbol_hover = TEXT_BLACK;
    combo.symbol_active = TEXT_BLACK;
    combo.sym_active = NK_SYMBOL_TRIANGLE_DOWN;
    combo.sym_hover = NK_SYMBOL_TRIANGLE_DOWN;
    combo.sym_normal = NK_SYMBOL_TRIANGLE_DOWN;
    combo.button.border = 0;
    combo.button.rounding = 0;
    combo.button.text_background = TRANSPARENT_COLOR;
    combo.button.text_normal = TEXT_WHITE;
    combo.button.text_hover = TEXT_BLACK;
    combo.button.text_active = TEXT_BLACK;
    combo.button.normal.data.color = TRANSPARENT_COLOR;
    combo.button.hover.data.color = TRANSPARENT_COLOR;
    combo.button.active.data.color = TRANSPARENT_COLOR;

    contextual_button.normal.data.color = TRANSPARENT_COLOR;
    contextual_button.hover.data.color = WHITE;
    contextual_button.active.data.color = ACTIVE_COLOR;
    contextual_button.text_normal = TEXT_WHITE;
    contextual_button.text_hover = TEXT_BLACK;
    contextual_button.text_active = TEXT_BLACK;
    contextual_button.border = 0;
    contextual_button.border_color = TRANSPARENT_COLOR;
    contextual_button.text_background = TRANSPARENT_COLOR;
    contextual_button.rounding = 0;

    button.text_background = TRANSPARENT_COLOR;
    button.text_normal = TEXT_WHITE;
    button.text_hover = TEXT_BLACK;
    button.text_active = TEXT_BLACK;
    button.normal.data.color = BACKGROUND_COLOR;
    button.hover.data.color = WHITE;
    button.active.data.color = ACTIVE_COLOR;
    button.border = 1.0f;
    button.border_color = BORDER_GREY;
    button.rounding = 0;
    // button.padding.x = 0;
    // button.padding.y = 0;

    checkbox.text_normal = TEXT_WHITE;
    checkbox.text_hover = TEXT_WHITE;
    checkbox.text_active = TEXT_WHITE;
    checkbox.text_background = TRANSPARENT_COLOR;
    checkbox.border = 1.0f;
    checkbox.border_color = TEXT_WHITE;
    checkbox.normal.data.color = BACKGROUND_COLOR;
    checkbox.hover.data.color = TEXT_GREY;
    checkbox.active.data.color = TEXT_WHITE;
    checkbox.cursor_hover.data.color = TEXT_WHITE;
    checkbox.cursor_normal.data.color = TEXT_WHITE;
    checkbox.padding.x = 0;
    checkbox.padding.y = 0;

    selectable.text_background = TRANSPARENT_COLOR;
    selectable.text_normal = TEXT_WHITE;
    selectable.text_hover = TEXT_BLACK;
    selectable.text_pressed = TEXT_BLACK;
    selectable.text_pressed_active = TEXT_BLACK;
    selectable.text_hover_active = TEXT_BLACK;
    selectable.text_normal_active = TEXT_BLACK;
    selectable.normal.data.color = BACKGROUND_COLOR;
    selectable.hover.data.color = WHITE;
    selectable.pressed.data.color = ACTIVE_COLOR;
    selectable.pressed_active.data.color = ACTIVE_COLOR;
    selectable.normal_active.data.color = ACTIVE_COLOR;
    selectable.hover_active.data.color = WHITE;
    selectable.rounding = 0;
    selectable.padding.x = 0;
    selectable.padding.y = 0;

    progress.border = 1.0f;
    progress.rounding = 0;
    progress.normal.data.color = TRANSPARENT_COLOR;
    progress.border_color = BORDER_GREY;
    progress.hover.data.color = TRANSPARENT_COLOR;
    progress.active.data.color = TRANSPARENT_COLOR;
    progress.cursor_normal.data.color = TEXT_WHITE;
    progress.cursor_hover.data.color = TEXT_WHITE;
    progress.cursor_active.data.color = TEXT_WHITE;
    progress.cursor_border = 0.0f;
    progress.cursor_border_color = TRANSPARENT_COLOR;
    progress.padding.x = 0;
    progress.padding.y = 0;

    /*customStyle->image = button;
    customStyle->image.normal.data.color = TRANSPARENT_COLOR;
    customStyle->image.hover.data.color = TRANSPARENT_COLOR;
    customStyle->image.active.data.color = TRANSPARENT_COLOR;

    customStyle->toggle = button;*/
}
