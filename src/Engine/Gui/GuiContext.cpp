#include "GuiContext.hpp"
#include "../Assets/Image.hpp"
#define NK_IMPLEMENTATION 1
#define NK_INCLUDE_DEFAULT_ALLOCATOR 1
#include <nuklear.h>

using namespace Engine;

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

struct GuiContext::CustomStyle {
    nk_style_button context;
};

GuiContext::GuiContext(const FontFamily& fontFamily, const int fontSize) :
    fontFamily{fontFamily},
    fontSize{fontSize},
    nk{std::make_unique<nk_context>()},
    custom{std::make_unique<CustomStyle>()} {

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
                             const Flags flags) {
    activeInput = false;

    if (flags & WindowFlag::Transparent) {
        nk_style_push_color(nk.get(), &nk->style.window.background, nk_rgba(0, 0, 0, 0));
        nk_style_push_style_item(
            nk.get(), &nk->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
    }

    if (flags & WindowFlag::HeaderSuccess) {
        nk_style_push_color(nk.get(), &nk->style.window.header.active.data.color, fromColor(Colors::secondary));
    } else if (flags & WindowFlag::HeaderDanger) {
        nk_style_push_color(nk.get(), &nk->style.window.header.active.data.color, fromColor(Colors::ternary));
    }

    // const auto* titleStr = flags & WindowFlag::Title ? title.c_str() : nullptr;

    const auto res = nk_begin_titled(nk.get(), id.c_str(), title.c_str(), nk_rect(pos.x, pos.y, size.x, size.y), flags);

    if (flags & WindowFlag::Transparent) {
        nk_style_pop_color(nk.get());
        nk_style_pop_style_item(nk.get());
    }
    if (flags & WindowFlag::HeaderSuccess || flags & WindowFlag::HeaderDanger) {
        nk_style_pop_color(nk.get());
    }

    return res == nk_true;
}

void GuiContext::windowEnd(const Flags flags) {
    nk_end(nk.get());
}

bool GuiContext::groupBegin(const std::string& name, const bool scrollbar) {
    auto flags = Flags{0} | WindowFlag::Border;
    if (!scrollbar) {
        flags = flags | WindowFlag::NoScrollbar;
    }

    return nk_group_begin_titled(nk.get(), name.c_str(), name.c_str(), flags) == nk_true;
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

bool GuiContext::imageToggle(const ImagePtr& img, bool& value, const Color4& color) {
    struct nk_image ni {};
    ni.handle.ptr = img.get();

    auto previous = value;
    if (nk_button_image_styled(nk.get(), &nk->style.button, ni) == nk_true) {
        value = true;
    }

    return previous != value;
}

bool GuiContext::imageToggleLabel(const ImagePtr& img, bool& value, const Color4& color, const std::string& text,
                                  const GuiTextAlign align) {
    struct nk_image ni {};
    ni.handle.ptr = img.get();

    const auto flags = static_cast<nk_flags>(align);
    auto previous = value;
    const auto len = static_cast<int>(text.size());

    if (button_image_text_styled(nk.get(), &nk->style.button, ni, text.c_str(), len, flags) == nk_true) {
        value = true;
    }

    return previous != value;
}

void GuiContext::groupEnd() {
    nk_group_end(nk.get());
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

bool GuiContext::button(const std::string& label, const ImagePtr& image) {
    if (image) {
        struct nk_image ni {};
        ni.handle.ptr = image.get();

        if (nk_button_image_label(nk.get(), ni, label.c_str(), NK_TEXT_ALIGN_LEFT)) {
            return true;
        }
    } else {
        if (nk_button_label(nk.get(), label.c_str())) {
            return true;
        }
    }

    return false;
}

bool GuiContext::buttonToggle(const std::string& label, bool& value) {
    if (value) {
        if (isHovered()) {
            nk_style_push_color(nk.get(), &nk->style.button.border_color, nk->style.button.hover.data.color);
        } else {
            nk_style_push_color(nk.get(), &nk->style.button.border_color, nk->style.button.active.data.color);
        }
    } else {
        if (isHovered()) {
            nk_style_push_color(nk.get(), &nk->style.button.border_color, nk->style.button.hover.data.color);
        } else {
            nk_style_push_color(nk.get(), &nk->style.button.border_color, nk->style.button.border_color);
        }
    }

    auto previous = value;
    if (nk_button_label(nk.get(), label.c_str())) {
        value = !value;
    }

    nk_style_pop_color(nk.get());

    return previous != value;
}

bool GuiContext::contextButton(const std::string& label, const ImagePtr& image) {
    struct nk_image ni {};
    ni.handle.ptr = image.get();

    auto& style = custom->context;

    const auto bounds = nk_widget_bounds(nk.get());

    struct nk_rect icon {};

    icon.w = bounds.h - 2.0f * style.padding.y;
    icon.h = bounds.h - 2.0f * style.padding.y;
    icon.y = bounds.y + (bounds.h / 2.0f) - (icon.h / 2.0f);
    icon.x = bounds.x + style.image_padding.x;

    const auto temp = style.padding.x;
    style.padding.x = icon.w + style.padding.x * 2.0f + style.image_padding.x;

    auto* color = &custom->context.text_normal;
    if (nk_input_is_mouse_hovering_rect(&nk->input, bounds) == nk_true) {
        color = &custom->context.text_hover;
    }

    const auto res = nk_button_label_styled(nk.get(), &style, label.c_str());

    style.padding.x = temp;

    nk_draw_image(&nk->current->buffer, icon, &ni, *color);

    return res == nk_true;
}

bool GuiContext::checkbox(const std::string& label, bool& value) {
    auto previous = value;
    value = nk_check_label(nk.get(), label.c_str(), value ? nk_true : nk_false) == nk_true;
    return previous != value;
}

void GuiContext::label(const std::string& label) {
    nk_label(nk.get(), label.c_str(), nk_text_align::NK_TEXT_ALIGN_LEFT);
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

    const auto bounds = nk_widget_bounds(nk.get());
    return nk_input_has_mouse_click_down_in_rect(&nk->input, nb, bounds, nk_true) == nk_true;
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
    inputEvents.emplace_back([=]() { nk_input_motion(nk.get(), pos.x, pos.y); });
    setDirty();
}

void GuiContext::eventMousePressed(const Vector2i& pos, const MouseButton button) {
    inputEvents.emplace_back([=]() { nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, true); });
    setDirty();
}

void GuiContext::eventMouseReleased(const Vector2i& pos, const MouseButton button) {
    inputEvents.emplace_back([=]() { nk_input_button(nk.get(), toNkButtons(button), pos.x, pos.y, false); });
    setDirty();
}

void GuiContext::eventMouseScroll(int xscroll, const int yscroll) {
    inputEvents.emplace_back([=]() { nk_input_scroll(nk.get(), nk_vec2(xscroll, yscroll)); });
    setDirty();
}

void GuiContext::eventKeyPressed(const Key key, const Modifiers modifiers) {
    const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        inputEvents.emplace_back([=]() { nk_input_key(nk.get(), toNkKeys(key), true); });
        setDirty();
    }
}

void GuiContext::eventKeyReleased(const Key key, const Modifiers modifiers) {
    const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        inputEvents.emplace_back([=]() { nk_input_key(nk.get(), toNkKeys(key), false); });
        setDirty();
    }
}

void GuiContext::eventCharTyped(const uint32_t code) {
    inputEvents.emplace_back([=]() { nk_input_unicode(nk.get(), code); });
    setDirty();
}

static const auto BLACK = HEX(0x000000ff);
static const auto PRIMARY_COLOR = fromColor(Colors::primary);
static const auto WHITE = HEX(0xe5e5e3ff);
static const auto TEXT_WHITE = fromColor(Colors::text);
static const auto TEXT_BLACK = HEX(0x030303ff);
static const auto TEXT_GREY = HEX(0x393939ff);
static const auto BACKGROUND_COLOR = fromColor(Colors::background);
static const auto BORDER_GREY = HEX(0x202020ff);
static const auto TRANSPARENT_COLOR = HEX(0x00000000);
static const auto ACTIVE_COLOR = PRIMARY_COLOR;

void GuiContext::applyTheme() {
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
    window.group_padding = nk_vec2(padding, padding);
    window.padding = nk_vec2(padding, padding);
    window.group_border = 1.0f;
    window.background = BACKGROUND_COLOR;
    window.min_row_height_padding = 0;
    window.combo_border_color = ACTIVE_COLOR;
    window.combo_border = 1.0f;
    window.group_border_color = BORDER_GREY;

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

    custom->context = button;
    custom->context.border = 0;
    custom->context.text_alignment = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;

    option.text_background = TRANSPARENT_COLOR;
    option.text_normal = TEXT_WHITE;
    option.text_hover = TEXT_BLACK;
    option.text_active = TEXT_BLACK;
    option.normal.data.color = BACKGROUND_COLOR;
    option.hover.data.color = WHITE;
    option.active.data.color = ACTIVE_COLOR;
    option.border = 0.0f;
    option.border_color = BORDER_GREY;

    edit.normal.data.color = BACKGROUND_COLOR;
    edit.hover.data.color = BACKGROUND_COLOR;
    edit.active.data.color = BACKGROUND_COLOR;
    edit.border = 1.0f;
    edit.border_color = BORDER_GREY;
    edit.text_normal = TEXT_WHITE;
    edit.text_hover = TEXT_WHITE;
    edit.text_active = ACTIVE_COLOR;
    edit.cursor_normal = ACTIVE_COLOR;
    edit.cursor_hover = ACTIVE_COLOR;
    edit.cursor_text_normal = TEXT_BLACK;
    edit.cursor_text_hover = TEXT_BLACK;
    // edit.selected_normal = ACTIVE_COLOR;
    // edit.selected_hover = ACTIVE_COLOR;

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
