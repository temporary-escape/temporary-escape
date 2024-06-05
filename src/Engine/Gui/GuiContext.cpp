#include "GuiContext.hpp"
#include "../Assets/Image.hpp"
#define NK_IMPLEMENTATION 1
#define NK_INCLUDE_DEFAULT_ALLOCATOR 1
#include <nuklear.h>
#include <utf8.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

void draw_button_text_image(struct nk_command_buffer* out, const struct nk_rect* bounds, const struct nk_rect* label,
                            const struct nk_rect* image, nk_flags state, const struct nk_style_button* style,
                            const char* str, int len, const struct nk_user_font* font, const struct nk_image* img,
                            nk_flags align) {
    struct nk_text text;
    const struct nk_style_item* background;
    background = nk_draw_button(out, bounds, state, style);

    /* select correct colors */
    if (background->type == NK_STYLE_ITEM_COLOR)
        text.background = background->data.color;
    else
        text.background = style->text_background;
    if (state & NK_WIDGET_STATE_HOVER)
        text.text = style->text_hover;
    else if (state & NK_WIDGET_STATE_ACTIVED)
        text.text = style->text_active;
    else
        text.text = style->text_normal;

    text.padding = nk_vec2(0, 0);
    nk_widget_text(out, *label, str, len, &text, align, font);
    nk_draw_image(out, *image, img, nk_white);
}

nk_bool do_button_text_image(nk_flags* state, struct nk_command_buffer* out, struct nk_rect bounds, struct nk_image img,
                             const char* str, int len, nk_flags align, enum nk_button_behavior behavior,
                             const struct nk_style_button* style, const struct nk_user_font* font,
                             const struct nk_input* in) {
    int ret;
    struct nk_rect icon;
    struct nk_rect content;

    NK_ASSERT(style);
    NK_ASSERT(state);
    NK_ASSERT(font);
    NK_ASSERT(out);
    if (!out || !font || !style || !str)
        return nk_false;

    ret = nk_do_button(state, out, bounds, style, in, behavior, &content);
    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    if (style->draw_begin)
        style->draw_begin(out, style->userdata);
    draw_button_text_image(out, &bounds, &content, &icon, *state, style, str, len, font, &img, align);
    if (style->draw_end)
        style->draw_end(out, style->userdata);
    return ret;
}

nk_bool button_image_text_styled(struct nk_context* ctx, const struct nk_style_button* style, struct nk_image img,
                                 const char* text, int len, nk_flags align) {
    struct nk_window* win;
    struct nk_panel* layout;
    const struct nk_input* in;

    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state)
        return 0;
    in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    return do_button_text_image(&ctx->last_widget_state,
                                &win->buffer,
                                bounds,
                                img,
                                text,
                                len,
                                align,
                                ctx->button_behavior,
                                style,
                                ctx->style.font,
                                in);
}

static Color4 asColor(const nk_color& color) {
    return {static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f,
            static_cast<float>(color.a) / 255.0f};
}

static nk_color HEX(const uint32_t v) {
    return nk_rgba((v & 0xFF000000) >> 24, (v & 0x00FF0000) >> 16, (v & 0x0000FF00) >> 8, (v & 0x000000FF) >> 0);
}

static constexpr nk_color fromColor(const Color4& color) {
    return nk_rgba(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

static float getTextWidth(const nk_handle handle, const float height, const char* str, const int len) {
    if (len == 0) {
        return 0.0f;
    }

    auto& font = *static_cast<const FontFamily*>(handle.ptr);

    auto it = str;
    const auto code = utf8::next(it, str + len);
    if (it == str + len) {
        return font.getGlyphAdvance(code, height);
    } else {
        return font.getBounds({str, static_cast<size_t>(len)}, height).x - 1.0f;
    }
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

struct GuiContext::CustomStyle {
    nk_style_button context;
    nk_style_button toggle;
};

GuiContext::GuiContext(const FontFamily& fontFamily, const int fontSize) :
    fontSize{fontSize}, nk{std::make_unique<nk_context>()}, custom{std::make_unique<CustomStyle>()} {

    font = std::make_unique<nk_user_font>();
    font->height = static_cast<float>(fontSize);
    font->width = &getTextWidth;
    font->userdata.ptr = const_cast<void*>(reinterpret_cast<const void*>(&fontFamily));

    nk_init_default(nk.get(), font.get());
    applyTheme();
}

GuiContext::~GuiContext() {
    if (nk) {
        nk_free(nk.get());
    }
}

const FontFamily& Engine::GuiContext::getFont() const {
    return *static_cast<const FontFamily*>(font->userdata.ptr);
}

void GuiContext::update() {
}

void GuiContext::render(Canvas& canvas) {
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
            const auto c = reinterpret_cast<const struct nk_command_image*>(cmd);
            const auto& color = asColor(c->col);
            const auto image = reinterpret_cast<Image*>(c->img.handle.ptr);
            if (image) {
                canvas.drawImage({static_cast<float>(c->x), static_cast<float>(c->y)},
                                 {static_cast<float>(c->w), static_cast<float>(c->h)},
                                 *image,
                                 color);
            }
            break;
        }
        default: {
            break;
        }
        }
    }
    nk_clear(nk.get());
}

bool GuiContext::windowBegin(const std::string& id, const std::string& title, const Vector2& pos, const Vector2& size,
                             const WindowOptions& options) {
    activeInput = false;

    auto bgColor = nk->style.window.background;
    bgColor.a = options.opacity * 255.0f;

    nk->style.window.background = bgColor;
    nk->style.window.fixed_background = nk_style_item_color(bgColor);

    Color4 titleColor = Colors::primary;
    Color4 headerColor = Colors::background;
    Color4 borderColor = Colors::primary;

    if (options.flags & WindowFlag::HeaderSuccess) {
        titleColor = Colors::black;
        headerColor = Colors::success;
        borderColor = Colors::success;
    } else if (options.flags & WindowFlag::HeaderDanger) {
        titleColor = Colors::black;
        headerColor = Colors::danger;
        borderColor = Colors::success;
    } else if (options.flags & WindowFlag::HeaderPrimary) {
        titleColor = Colors::black;
        headerColor = Colors::primary;
        borderColor = Colors::primary;
    }

    nk->style.window.header.active.data.color = fromColor(headerColor);
    nk->style.window.header.hover.data.color = fromColor(headerColor);
    nk->style.window.header.normal.data.color = fromColor(headerColor);
    nk->style.window.header.label_active = fromColor(titleColor);
    nk->style.window.header.label_hover = fromColor(titleColor);
    nk->style.window.header.label_normal = fromColor(titleColor);
    nk->style.window.border_color = fromColor(borderColor);

    auto flags = options.flags;
    if (!inputEnabled) {
        flags |= WindowFlag::NonInteractive;
    }

    const auto res = nk_begin_titled(nk.get(), id.c_str(), title.c_str(), nk_rect(pos.x, pos.y, size.x, size.y), flags);

    return res == nk_true;
}

void GuiContext::windowEnd(const Flags flags) {
    nk_end(nk.get());
}

bool GuiContext::isWindowClosed() const {
    if (!nk->current) {
        return false;
    }
    return nk->current->layout->flags & NK_WINDOW_HIDDEN;
}

void Engine::GuiContext::setInputEnabled(const bool value) {
    inputEnabled = value;
}

void Engine::GuiContext::setFocused(const std::string& id) {
    nk_window_set_focus(nk.get(), id.c_str());
}

bool GuiContext::groupBegin(const std::string& name, const bool scrollbar, const bool border,
                            const Color4& borderColor) {
    Flags flags{0};
    if (!scrollbar) {
        flags = flags | WindowFlag::NoScrollbar;
    }
    if (border) {
        flags = flags | WindowFlag::Border;
        // nk_style_push_vec2(nk.get(), &nk->style.window.group_padding, nk_vec2(0.0f, 0.0f));
        // nk_style_push_color(nk.get(), &nk->style.window.group_border_color, fromColor(borderColor));
        nk->style.window.group_border_color = fromColor(borderColor);
    }

    nk_style_push_color(nk.get(), &nk->style.window.background, nk_rgba(0, 0, 0, 0));
    nk_style_push_style_item(nk.get(), &nk->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

    const auto res = nk_group_begin_titled(nk.get(), name.c_str(), name.c_str(), flags) == nk_true;

    nk_style_pop_color(nk.get());
    nk_style_pop_style_item(nk.get());

    // if (border) {
    //  nk_style_pop_vec2(nk.get());
    //  nk_style_pop_color(nk.get());
    //}

    return res;
}

void GuiContext::groupEnd() {
    nk_group_end(nk.get());
    nk->style.window.group_border_color = fromColor(Colors::border);
}

bool GuiContext::comboBegin(const Vector2& size, const std::string& label) {
    struct nk_vec2 s {
        size.x, size.y,
    };
    const auto height = nk_widget_height(nk.get());

    return nk_combo_begin_label(nk.get(), label.c_str(), s) == nk_true;
}

void GuiContext::comboEnd() {
    nk_combo_end(nk.get());
}

bool GuiContext::comboItem(const std::string& label) {
    const auto height = nk_widget_height(nk.get());

    nk_layout_row_dynamic(nk.get(), height, 1);

    return nk_combo_item_label(nk.get(), label.c_str(), NK_TEXT_ALIGN_LEFT) == nk_true;
}

void GuiContext::progress(const float value, const float max) {
    auto current = static_cast<nk_size>(value);
    nk_progress(nk.get(), &current, static_cast<nk_size>(max), NK_FIXED);
}

void GuiContext::progress(const float value, const float max, const GuiStyleProgress& style, const float height) {
    struct nk_rect bounds {};
    nk_widget(&bounds, nk.get());

    if (height > 0.0f) {
        bounds.y += bounds.h / 2.0f - height / 2.0f;
        bounds.h = height;
    }

    const auto* borderColor = isHovered() ? &style.border.hover : &style.border.normal;
    const auto* rectColor = isHovered() ? &style.bar.hover : &style.bar.normal;

    nk_stroke_rect(&nk->current->buffer, bounds, 0.0f, 1.0f, fromColor(*borderColor));

    bounds.x += 1;
    bounds.y += 1;
    bounds.w -= 2;
    bounds.h -= 2;
    bounds.w = bounds.w * (value / max);
    nk_fill_rect(&nk->current->buffer, bounds, 0.0f, fromColor(*rectColor));
}

void GuiContext::image(const ImagePtr& img, const Color4& color) {
    struct nk_image ni {};
    ni.handle.ptr = img.get();
    nk_image_color(nk.get(), ni, fromColor(color));
}

bool GuiContext::imageToggle(const ImagePtr& img, bool& value, const GuiStyleButton& style, const Color4& color) {
    struct nk_image ni {};
    ni.handle.ptr = img.get();

    auto previous = value;
    if (nk_button_image_styled(nk.get(), style.nk.get(), ni) == nk_true) {
        value = true;
    }

    return previous != value;
}

bool GuiContext::imageToggleLabel(const ImagePtr& img, bool& value, const GuiStyleButton& style, const Color4& color,
                                  const std::string& text, const GuiTextAlign align) {
    struct nk_image ni {};
    ni.handle.ptr = img.get();

    const auto flags = static_cast<nk_flags>(align);
    auto previous = value;
    const auto len = static_cast<int>(text.size());

    if (button_image_text_styled(nk.get(), style.nk.get(), ni, text.c_str(), len, flags) == nk_true) {
        value = true;
    }

    return previous != value;
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

void GuiContext::layoutTemplateBegin(const float height) {
    nk_layout_row_template_begin(nk.get(), height);
}

void GuiContext::layoutTemplatePushDynamic() {
    nk_layout_row_template_push_dynamic(nk.get());
}

void GuiContext::layoutTemplatePushVariable(const float width) {
    nk_layout_row_template_push_variable(nk.get(), width);
}

void GuiContext::layoutTemplatePushStatic(const float width) {
    nk_layout_row_template_push_static(nk.get(), width);
}

void GuiContext::layoutTemplateEnd() {
    nk_layout_row_template_end(nk.get());
}

void GuiContext::skip() {
    struct nk_rect bounds {};
    nk_widget(&bounds, nk.get());
}

bool GuiContext::button(const std::string& label, const GuiStyleButton& style, const ImagePtr& image) {
    nk->style.button = *style.nk;

    if (image) {
        struct nk_image ni {};
        ni.handle.ptr = image.get();

        if (nk_button_image_text_styled(
                nk.get(), style.nk.get(), ni, label.c_str(), label.size(), NK_TEXT_ALIGN_LEFT)) {
            return true;
        }
    } else {
        if (nk_button_text_styled(nk.get(), style.nk.get(), label.c_str(), label.size())) {
            return true;
        }
    }

    return false;
}

bool GuiContext::buttonToggle(const std::string& label, bool& value) {
    if (value) {
        custom->toggle.border_color = fromColor(Colors::primary);
    } else {
        custom->toggle.border_color = fromColor(Colors::white);
    }

    auto previous = value;
    if (nk_button_text_styled(nk.get(), &custom->toggle, label.c_str(), label.size())) {
        value = !value;
    }

    return previous != value;
}

bool GuiContext::contextButton(const std::string& label, const ImagePtr& imageLeft, const ImagePtr& imageRight) {
    struct nk_image ni {};

    auto& style = custom->context;

    const auto bounds = nk_widget_bounds(nk.get());

    struct nk_rect icon {};

    const auto temp = style.padding.x;
    if (imageLeft) {
        icon.w = bounds.h - 2.0f * style.padding.y;
        icon.h = bounds.h - 2.0f * style.padding.y;
        icon.y = bounds.y + (bounds.h / 2.0f) - (icon.h / 2.0f);
        icon.x = bounds.x + style.image_padding.x;

        style.padding.x = icon.w + style.padding.x * 2.0f + style.image_padding.x;
    }

    auto* color = &custom->context.text_normal;
    if (nk_input_is_mouse_hovering_rect(&nk->input, bounds) == nk_true) {
        color = &custom->context.text_hover;
    }

    const auto res = nk_button_label_styled(nk.get(), &style, label.c_str());

    style.padding.x = temp;

    if (imageLeft) {
        ni.handle.ptr = imageLeft.get();
        nk_draw_image(&nk->current->buffer, icon, &ni, *color);
    }

    if (imageRight) {
        icon.w = bounds.h - 2.0f * style.padding.y;
        icon.h = bounds.h - 2.0f * style.padding.y;
        icon.y = bounds.y + (bounds.h / 2.0f) - (icon.h / 2.0f);
        icon.x = bounds.x + bounds.w - icon.w - style.padding.x;

        ni.handle.ptr = imageRight.get();
        nk_draw_image(&nk->current->buffer, icon, &ni, *color);
    }

    return res == nk_true;
}

bool GuiContext::checkbox(const std::string& label, bool& value) {
    auto previous = value;
    value = nk_check_label(nk.get(), label.c_str(), value ? nk_true : nk_false) == nk_true;
    return previous != value;
}

void GuiContext::label(const std::string& value, const Color4& color) {
    const auto flags = nk_text_align::NK_TEXT_ALIGN_LEFT | nk_text_align::NK_TEXT_ALIGN_MIDDLE;
    nk_text_colored(nk.get(), value.c_str(), value.size(), flags, fromColor(color));
}

void GuiContext::text(const std::string& value) {
    const auto flags = nk_text_align::NK_TEXT_ALIGN_LEFT | nk_text_align::NK_TEXT_ALIGN_TOP;
    nk_text_colored(nk.get(), value.c_str(), value.size(), flags, nk->style.text.color);
}

bool GuiContext::textInput(std::string& text, size_t max) {
    if (editBuffer.size() < max + 1) {
        editBuffer.resize(max + 1);
    }
    std::memcpy(editBuffer.data(), text.data(), std::min(text.size(), max));
    editBuffer[text.size()] = '\0';

    auto len = static_cast<int>(std::min(text.size(), max));
    auto modified = false;

    const auto state = nk_edit_string(nk.get(), NK_EDIT_SIMPLE, editBuffer.data(), &len, max, nk_filter_default);
    if (len != text.size()) {
        text.resize(len);
        std::memcpy(text.data(), editBuffer.data(), text.size());
        modified = true;
    }

    activeInput = !(state & NK_WIDGET_STATE_ACTIVE);

    return modified;
}

void GuiContext::tooltip(const std::string& text) {
    if (isHovered()) {
        nk_tooltip(nk.get(), text.c_str());
    }
}

bool GuiContext::isHovered() const {
    // return nk_widget_is_hovered(ctx.get()) == nk_true;
    const auto bounds = nk_widget_bounds(nk.get());
    return nk_input_is_mouse_hovering_rect(&nk->input, bounds) == nk_true;
}

bool GuiContext::isMouseDown(const MouseButton button) const {
    nk_buttons nb;
    switch (button) {
    case MouseButton::Left: {
        nb = NK_BUTTON_LEFT;
        break;
    }
    case MouseButton::Right: {
        nb = NK_BUTTON_RIGHT;
        break;
    }
    case MouseButton::Middle: {
        nb = NK_BUTTON_MIDDLE;
        break;
    }
    default: {
        return false;
    }
    }

    return nk_input_is_mouse_down(&nk->input, nb);

    // const auto bounds = nk_widget_bounds(nk.get());
    // return nk_input_has_mouse_click_down_in_rect(&nk->input, nb, bounds, nk_true) == nk_true;
}

Vector2 GuiContext::getWidgetSize() const {
    return {
        nk_widget_width(nk.get()),
        nk_widget_height(nk.get()),
    };
}

Vector2 GuiContext::getPadding() const {
    return {
        nk->style.window.padding.x,
        nk->style.window.padding.y,
    };
}

void GuiContext::eventMouseMoved(const Vector2i& pos) {
    // inputEvents.emplace_back([=]() { nk_input_motion(nk.get(), pos.x, pos.y); });
    nk_input_motion(nk.get(), pos.x, pos.y);
}

void GuiContext::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    // inputEvents.emplace_back([=]() { nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, true); });
    nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, true);
}

void GuiContext::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    // inputEvents.emplace_back([=]() { nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, false); });
    nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, false);
}

void GuiContext::eventMouseScroll(int xscroll, const int yscroll) {
    // inputEvents.emplace_back([=]() { nk_input_scroll(nk.get(), nk_vec2(xscroll, yscroll)); });
    nk_input_scroll(nk.get(), nk_vec2(xscroll, yscroll));
}

void GuiContext::eventKeyPressed(const Key key, const Modifiers modifiers) {
    const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        // inputEvents.emplace_back([=]() { nk_input_key(nk.get(), toNkKeys(key), true); });
        nk_input_key(nk.get(), toNkKeys(key), true);
    }
}

void GuiContext::eventKeyReleased(const Key key, const Modifiers modifiers) {
    const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        // inputEvents.emplace_back([=]() { nk_input_key(nk.get(), toNkKeys(key), false); });
        nk_input_key(nk.get(), toNkKeys(key), false);
    }
}

void GuiContext::eventCharTyped(const uint32_t code) {
    nk_input_unicode(nk.get(), code);
    // inputEvents.emplace_back([=]() { nk_input_unicode(nk.get(), code); });
}

void Engine::GuiContext::eventInputBegin() {
    nk_input_begin(nk.get());
}

void Engine::GuiContext::eventInputEnd() {
    nk_input_end(nk.get());
}

void Engine::GuiContext::setPadding(const float value) {
    nk->style.window.padding = {value, value};
}

static const auto TRANSPARENT_COLOR = fromColor(Colors::transparent);

void GuiContext::applyTheme() {
    static constexpr float border = 2.0f;

    auto& window = nk->style.window;
    auto& button = nk->style.button;
    auto& edit = nk->style.edit;
    auto& checkbox = nk->style.checkbox;
    auto& selectable = nk->style.selectable;
    auto& text = nk->style.text;
    auto& combo = nk->style.combo;
    auto& contextual_button = nk->style.contextual_button;
    auto& progress = nk->style.progress;
    auto& option = nk->style.option;

    text.color = fromColor(Colors::text);
    text.padding.x = 0;
    text.padding.y = 2;

    // window.background = PRIMARY_COLOR;
    window.rounding = 0;
    window.fixed_background.data.color = fromColor(Colors::background);
    window.header.normal.data.color = fromColor(Colors::background);
    window.header.hover.data.color = fromColor(Colors::background);
    window.header.active.data.color = fromColor(Colors::background);
    window.header.close_button.normal.data.color = fromColor(Colors::dangerBackground);
    window.header.close_button.hover.data.color = fromColor(Colors::danger);
    window.header.close_button.active.data.color = fromColor(Colors::danger);
    window.header.close_button.text_normal = fromColor(Colors::danger);
    window.header.close_button.text_hover = fromColor(Colors::white);
    window.header.close_button.text_active = fromColor(Colors::white);
    window.header.close_button.border = 1.0f;
    window.header.close_button.border_color = fromColor(Colors::danger);
    window.header.label_normal = fromColor(Colors::primary);
    window.header.label_hover = fromColor(Colors::primary);
    window.header.label_active = fromColor(Colors::primary);
    window.header.padding = nk_vec2(3, 3);
    window.group_padding = nk_vec2(padding, padding);
    window.padding = nk_vec2(padding, padding);
    window.group_border = 2.0f;
    window.background = fromColor(Colors::background);
    window.min_row_height_padding = 0;
    window.combo_border = 2.0f;

    window.border = border;
    window.combo_border = border;
    window.contextual_border = border;
    window.menu_border = border;
    window.group_border = border;
    window.tooltip_border = border;
    window.popup_border = border;

    window.border_color = fromColor(Colors::primary);
    window.popup_border_color = fromColor(Colors::primary);
    window.combo_border_color = fromColor(Colors::primary);
    window.contextual_border_color = fromColor(Colors::primary);
    window.menu_border_color = fromColor(Colors::primary);
    window.group_border_color = fromColor(Colors::white);
    window.tooltip_border_color = fromColor(Colors::border);

    combo.border = border;
    combo.border_color = fromColor(Colors::border);
    combo.rounding = 0;
    combo.normal.data.color = fromColor(Colors::background);
    combo.hover.data.color = fromColor(Colors::white);
    combo.active.data.color = fromColor(Colors::primary);
    // combo.button_padding = nk_vec2(0, 0);
    // combo.content_padding = nk_vec2(0, 0);
    combo.label_normal = fromColor(Colors::text);
    combo.label_hover = fromColor(Colors::black);
    combo.label_active = fromColor(Colors::black);
    combo.symbol_normal = fromColor(Colors::text);
    combo.symbol_hover = fromColor(Colors::black);
    combo.symbol_active = fromColor(Colors::black);
    combo.sym_active = NK_SYMBOL_TRIANGLE_DOWN;
    combo.sym_hover = NK_SYMBOL_TRIANGLE_DOWN;
    combo.sym_normal = NK_SYMBOL_TRIANGLE_DOWN;
    combo.button.border = 0;
    combo.button.rounding = 0;
    combo.button.text_background = TRANSPARENT_COLOR;
    combo.button.text_normal = fromColor(Colors::text);
    combo.button.text_hover = fromColor(Colors::black);
    combo.button.text_active = fromColor(Colors::black);
    combo.button.normal.data.color = TRANSPARENT_COLOR;
    combo.button.hover.data.color = TRANSPARENT_COLOR;
    combo.button.active.data.color = TRANSPARENT_COLOR;

    contextual_button.normal.data.color = TRANSPARENT_COLOR;
    contextual_button.hover.data.color = fromColor(Colors::white);
    contextual_button.active.data.color = fromColor(Colors::primary);
    contextual_button.text_normal = fromColor(Colors::text);
    contextual_button.text_hover = fromColor(Colors::black);
    contextual_button.text_active = fromColor(Colors::black);
    contextual_button.border = 0;
    contextual_button.border_color = TRANSPARENT_COLOR;
    contextual_button.text_background = TRANSPARENT_COLOR;
    contextual_button.rounding = 0;

    button.text_background = TRANSPARENT_COLOR;
    button.text_normal = fromColor(Colors::background);
    button.text_hover = fromColor(Colors::background);
    button.text_active = fromColor(Colors::primaryBackground);
    button.normal.data.color = fromColor(Colors::white);
    button.hover.data.color = fromColor(Colors::primary);
    button.active.data.color = fromColor(Colors::primary);
    button.border = border;
    button.border_color = fromColor(Colors::white);
    button.rounding = 0;
    // button.padding.x = 0;
    // button.padding.y = 0;

    custom->toggle = button;
    custom->toggle.text_background = TRANSPARENT_COLOR;
    custom->toggle.text_normal = fromColor(Colors::white);
    custom->toggle.text_hover = fromColor(Colors::primary);
    custom->toggle.text_active = fromColor(Colors::primary);
    custom->toggle.normal.data.color = fromColor(Colors::background);
    custom->toggle.hover.data.color = fromColor(Colors::background);
    custom->toggle.active.data.color = fromColor(Colors::primaryBackground);
    custom->toggle.border = border;
    custom->toggle.border_color = fromColor(Colors::white);
    custom->toggle.rounding = 0;

    custom->context = button;
    custom->context.text_background = TRANSPARENT_COLOR;
    custom->context.text_normal = fromColor(Colors::white);
    custom->context.text_hover = fromColor(Colors::black);
    custom->context.text_active = fromColor(Colors::black);
    custom->context.normal.data.color = fromColor(Colors::transparent);
    custom->context.hover.data.color = fromColor(Colors::white);
    custom->context.active.data.color = fromColor(Colors::primary);
    custom->context.border = 0.0f;
    custom->context.border_color = fromColor(Colors::transparent);
    custom->context.rounding = 0;
    custom->context.text_alignment = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;

    option.text_background = TRANSPARENT_COLOR;
    option.text_normal = fromColor(Colors::text);
    option.text_hover = fromColor(Colors::black);
    option.text_active = fromColor(Colors::black);
    option.normal.data.color = fromColor(Colors::background);
    option.hover.data.color = fromColor(Colors::white);
    option.active.data.color = fromColor(Colors::primary);
    option.border = 0.0f;
    option.border_color = fromColor(Colors::border);

    edit.normal.data.color = fromColor(Colors::background);
    edit.hover.data.color = fromColor(Colors::background);
    edit.active.data.color = fromColor(Colors::background);
    edit.border = border;
    edit.border_color = fromColor(Colors::border);
    edit.text_normal = fromColor(Colors::text);
    edit.text_hover = fromColor(Colors::text);
    edit.text_active = fromColor(Colors::primary);
    edit.cursor_normal = fromColor(Colors::primary);
    edit.cursor_hover = fromColor(Colors::primary);
    edit.cursor_text_normal = fromColor(Colors::black);
    edit.cursor_text_hover = fromColor(Colors::black);
    // edit.selected_normal = ACTIVE_COLOR;
    // edit.selected_hover = ACTIVE_COLOR;

    checkbox.text_normal = fromColor(Colors::text);
    checkbox.text_hover = fromColor(Colors::text);
    checkbox.text_active = fromColor(Colors::text);
    checkbox.text_background = TRANSPARENT_COLOR;
    checkbox.border = border;
    checkbox.border_color = fromColor(Colors::text);
    checkbox.normal.data.color = fromColor(Colors::background);
    checkbox.hover.data.color = fromColor(Colors::textGray);
    checkbox.active.data.color = fromColor(Colors::text);
    checkbox.cursor_hover.data.color = fromColor(Colors::text);
    checkbox.cursor_normal.data.color = fromColor(Colors::text);
    checkbox.padding.x = 0;
    checkbox.padding.y = 0;

    selectable.text_background = TRANSPARENT_COLOR;
    selectable.text_normal = fromColor(Colors::text);
    selectable.text_hover = fromColor(Colors::black);
    selectable.text_pressed = fromColor(Colors::black);
    selectable.text_pressed_active = fromColor(Colors::black);
    selectable.text_hover_active = fromColor(Colors::black);
    selectable.text_normal_active = fromColor(Colors::black);
    selectable.normal.data.color = fromColor(Colors::background);
    selectable.hover.data.color = fromColor(Colors::white);
    selectable.pressed.data.color = fromColor(Colors::primary);
    selectable.pressed_active.data.color = fromColor(Colors::primary);
    selectable.normal_active.data.color = fromColor(Colors::primary);
    selectable.hover_active.data.color = fromColor(Colors::white);
    selectable.rounding = 0;
    selectable.padding.x = 0;
    selectable.padding.y = 0;

    progress.border = border;
    progress.rounding = 0;
    progress.normal.data.color = TRANSPARENT_COLOR;
    progress.border_color = fromColor(Colors::border);
    progress.hover.data.color = TRANSPARENT_COLOR;
    progress.active.data.color = TRANSPARENT_COLOR;
    progress.cursor_normal.data.color = fromColor(Colors::text);
    progress.cursor_hover.data.color = fromColor(Colors::text);
    progress.cursor_active.data.color = fromColor(Colors::text);
    progress.cursor_border = 0.0f;
    progress.cursor_border_color = TRANSPARENT_COLOR;
    progress.padding.x = 0;
    progress.padding.y = 0;
}

GuiStyleButton::GuiStyleButton(const GuiStyleColor& color, const GuiStyleColor& text, const Color4& border) {
    nk = std::make_unique<nk_style_button>();
    nk->text_background = fromColor(hexColor(0x00000000));
    nk->text_normal = fromColor(text.normal);
    nk->text_hover = fromColor(text.hover);
    nk->text_active = fromColor(text.active);
    nk->normal.data.color = fromColor(color.normal);
    nk->hover.data.color = fromColor(color.hover);
    nk->active.data.color = fromColor(color.active);
    nk->border = border.a > 0.01f ? 2.0f : 0.0f;
    nk->border_color = fromColor(border);
    nk->rounding = 0;
    nk->text_alignment = NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED;
}

GuiStyleButton::~GuiStyleButton() = default;
