#include "nuklear.hpp"
#include "theme.hpp"
#define NK_IMPLEMENTATION 1
#define NK_INCLUDE_DEFAULT_ALLOCATOR 1
#include <nuklear.h>

using namespace Engine;

static Color4 asColor(const nk_color& color) {
    return {static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f, static_cast<float>(color.a) / 255.0f};
}

static float getTextWidth(const nk_handle handle, const float height, const char* str, const int len) {
    auto& font = *static_cast<const FontFace*>(handle.ptr);
    return font.getBounds({str, static_cast<size_t>(len)}, height).x;
}

Nuklear::Nuklear(Canvas& canvas, const FontFace& fontFace, float fontSize) :
    canvas{canvas}, ctx{std::make_unique<nk_context>()}, ctxFont{std::make_unique<nk_user_font>()} {
    ctxFont->height = fontSize;
    ctxFont->width = &getTextWidth;
    ctxFont->userdata.ptr = const_cast<void*>(reinterpret_cast<const void*>(&fontFace));
    nk_init_default(ctx.get(), ctxFont.get());

    applyTheme();
}

Nuklear::~Nuklear() {
    nk_free(ctx.get());
}

void Nuklear::begin(const Vector2i& viewport) {
    lastViewportValue = viewport;
    windowsBounds.clear();
    input();
}

void Nuklear::end() {
    render();
    nk_clear(ctx.get());
}

void Nuklear::draw(NuklearWindow& window) {
    window.draw(*this, getViewport());
}

void Nuklear::input() {
    nk_input_begin(ctx.get());
    for (const auto& event : inputEvents) {
        event();
    }
    inputEvents.clear();
    nk_input_end(ctx.get());
}

void Nuklear::render() {
    const struct nk_command* cmd;
    nk_foreach(cmd, ctx.get()) {
        switch (cmd->type) {
        case NK_COMMAND_SCISSOR: {
            const auto c = reinterpret_cast<const struct nk_command_scissor*>(cmd);
            // glScissor(c->x, viewport.y() - c->y + c->h, c->w, c->h);
            canvas.scissor({static_cast<float>(c->x), static_cast<float>(c->y)},
                           {static_cast<float>(c->w), static_cast<float>(c->h)});

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
            auto& font = *static_cast<const FontFace*>(c->font->userdata.ptr);
            canvas.text(Vector2(c->x, static_cast<float>(c->y) + c->height / 1.25f), &c->string[0], font,
                        c->font->height, asColor(c->foreground));
            break;
        }
        case NK_COMMAND_RECT: {
            const auto c = reinterpret_cast<const struct nk_command_rect*>(cmd);
            const auto p = Vector2{c->x, c->y} + Vector2{c->line_thickness / 2.0f};
            const auto s = Vector2{c->w, c->h} - Vector2{static_cast<float>(c->line_thickness)};
            canvas.rectOutline(p, s, asColor(c->color));
            break;
        }
        case NK_COMMAND_RECT_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_rect_filled*>(cmd);
            canvas.rect(Vector2(c->x, c->y), Vector2(c->w, c->h), asColor(c->color));
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
            const auto c = reinterpret_cast<const struct nk_command_image*>(cmd);
            /*const auto& color = asColor(c->col);
            const auto image = reinterpret_cast<Canvas2D::Image*>(c->img.handle.ptr);
            canvas.beginPath();
            canvas.rectImage({static_cast<float>(c->x), static_cast<float>(c->y)},
                             {static_cast<float>(c->w), static_cast<float>(c->h)}, *image, color);
            canvas.fill();
            canvas.closePath();*/
            break;
        }
        default: {
            break;
        }
        }
    }
}

bool Nuklear::beginWindow(const std::string& title, const Vector2& pos, const Vector2& size, const Flags flags) {
    if (nk_begin_titled(ctx.get(), title.c_str(), title.c_str(), nk_rect(pos.x, pos.y, size.x, size.y), flags)) {
        windowsBounds.emplace_back(pos, size);
        nk_window_set_position(ctx.get(), title.c_str(), nk_vec2(pos.x, pos.y));
        return true;
    }

    return false;
}

void Nuklear::endWindow() {
    nk_end(ctx.get());
}

void Nuklear::layoutDynamic(float height, int count) {
    nk_layout_row_dynamic(ctx.get(), height, count);
}

bool Nuklear::button(const std::string& text, const TextAlign align) {
    nk_style_button& style = ctx->style.button;
    style.text_alignment = static_cast<nk_flags>(align);

    return nk_button_label(ctx.get(), text.c_str()) == nk_true;
}

void Nuklear::label(const std::string& text) {
    nk_label(ctx.get(), text.c_str(), nk_text_align::NK_TEXT_ALIGN_LEFT);
}

void Nuklear::progress(float value) {
    auto current = static_cast<nk_size>(value * 100.0f);
    nk_progress(ctx.get(), &current, 100, nk_false);
}

void Nuklear::eventMouseMoved(const Vector2i& pos) {
    inputEvents.emplace_back([=]() { nk_input_motion(ctx.get(), pos.x, pos.y); });
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

void Nuklear::eventMousePressed(const Vector2i& pos, MouseButton button) {
    inputEvents.emplace_back([=]() { nk_input_button(ctx.get(), toNkButtons(button), pos.x, pos.y, true); });
}

void Nuklear::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    inputEvents.emplace_back([=]() { nk_input_button(ctx.get(), toNkButtons(button), pos.x, pos.y, false); });
}

void Nuklear::eventMouseScroll(int xscroll, int yscroll) {
    inputEvents.emplace_back([=]() { nk_input_scroll(ctx.get(), nk_vec2(xscroll, yscroll)); });
}

void Nuklear::eventKeyPressed(Key key, Modifiers modifiers) {
    const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        inputEvents.emplace_back([=]() { nk_input_key(ctx.get(), toNkKeys(key), true); });
    }
}

void Nuklear::eventKeyReleased(Key key, Modifiers modifiers) {
    const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        inputEvents.emplace_back([=]() { nk_input_key(ctx.get(), toNkKeys(key), false); });
    }
}

void Nuklear::eventCharTyped(uint32_t code) {
    inputEvents.emplace_back([=]() { nk_input_unicode(ctx.get(), code); });
}

static nk_color HEX(const uint32_t v) {
    return nk_rgba((v & 0xFF000000) >> 24, (v & 0x00FF0000) >> 16, (v & 0x0000FF00) >> 8, (v & 0x000000FF) >> 0);
}

static const auto PRIMARY_COLOR = HEX(0xe7000dff);
static const auto SECONDARY_COLOR = HEX(0xfbd90dff);
static const auto WHITE = HEX(0xe5e5e3ff);
static const auto TEXT_WHITE = HEX(0xd0d0d0ff);
static const auto TEXT_BLACK = HEX(0x030303ff);
static const auto TEXT_GREY = HEX(0x393939ff);
static const auto BACKGROUND_COLOR = HEX(0x0a0a0aff);
static const auto BORDER_GREY = HEX(0x363636ff);
static const auto TRANSPARENT_COLOR = HEX(0x00000000);
static const auto ACTIVE_COLOR = SECONDARY_COLOR;

void Nuklear::applyTheme() {
    auto& window = ctx->style.window;
    auto& button = ctx->style.button;
    auto& checkbox = ctx->style.checkbox;
    auto& selectable = ctx->style.selectable;
    auto& text = ctx->style.text;
    auto& combo = ctx->style.combo;
    auto& contextual_button = ctx->style.contextual_button;
    auto& progress = ctx->style.progress;

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
}
