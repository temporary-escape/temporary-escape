#include "GuiContext.hpp"
#include "../Assets/AssetManager.hpp"

#include <nuklear.h>

extern "C" {

NK_LIB void* nk_memcopy(void* dst0, const void* src0, nk_size length);
NK_LIB void nk_memset(void* ptr, int c0, nk_size size);

NK_LIB void* NK_MEMCPY(void* dst0, const void* src0, nk_size length) {
    return nk_memcopy(dst0, src0, length);
}
NK_LIB void NK_MEMSET(void* ptr, int c0, nk_size size) {
    return nk_memset(ptr, c0, size);
}

#include <nuklear_internal.h>

void cnk_draw_button_image(struct nk_context* ctx, struct nk_command_buffer* out, const struct nk_rect* bounds,
                           const struct nk_rect* content, nk_flags state, const struct nk_style_button* style,
                           const struct nk_image* img, const nk_bool active) {
    nk_draw_button(out, bounds, state | (active ? NK_WIDGET_STATE_ACTIVE : 0), style);

    NK_MEMCPY(ctx, ctx, sizeof(ctx));

    auto c = style->text_normal;
    if (active || nk_input_is_mouse_hovering_rect(&ctx->input, *bounds)) {
        if (state & NK_WIDGET_STATE_HOVER) {
            c = style->text_hover;
        } else if (active || state & NK_WIDGET_STATE_ACTIVE) {
            c = style->text_active;
        }
    }

    nk_draw_image(out, *content, img, c);
}

nk_bool cnk_do_button_image(struct nk_context* ctx, struct nk_command_buffer* out, struct nk_rect bounds,
                            struct nk_image img, enum nk_button_behavior b, const struct nk_style_button* style,
                            const struct nk_input* in, const nk_bool active) {
    int ret;
    struct nk_rect content;

    auto state = &ctx->last_widget_state;

    NK_ASSERT(state);
    NK_ASSERT(style);
    NK_ASSERT(out);
    if (!out || !style || !state)
        return nk_false;

    ret = nk_do_button(state, out, bounds, style, in, b, &content);
    content.x += style->image_padding.x;
    content.y += style->image_padding.y;
    content.w -= 2 * style->image_padding.x;
    content.h -= 2 * style->image_padding.y;

    if (style->draw_begin)
        style->draw_begin(out, style->userdata);
    cnk_draw_button_image(ctx, out, &bounds, &content, *state, style, &img, active);
    if (style->draw_end)
        style->draw_end(out, style->userdata);
    return ret;
}

nk_bool cnk_button_image_styled(struct nk_context* ctx, const struct nk_style_button* style, struct nk_image img,
                                const nk_bool active) {
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
    return cnk_do_button_image(ctx, &win->buffer, bounds, img, ctx->button_behavior, style, in, active);
}

nk_bool cnk_button_image(struct nk_context* ctx, struct nk_image img, const nk_bool active) {
    assert(ctx);
    if (!ctx)
        return 0;
    return cnk_button_image_styled(ctx, &ctx->style.button, img, active);
}
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

using namespace Engine;

static Color4 asColor(const nk_color& color) {
    return {static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f, static_cast<float>(color.a) / 255.0f};
}

static nk_color toNkColor(const Color4& color) {
    nk_color res;
    res.r = static_cast<nk_byte>(color.r * 255.0f);
    res.g = static_cast<nk_byte>(color.g * 255.0f);
    res.b = static_cast<nk_byte>(color.b * 255.0f);
    res.a = static_cast<nk_byte>(color.a * 255.0f);
    return res;
}

static Vector2 asVec(const struct nk_vec2& vec) {
    return {vec.x, vec.y};
}

static Vector2i asVec(const struct nk_vec2i& vec) {
    return {vec.x, vec.y};
}

static nk_buttons asButton(const MouseButton button) {
    switch (button) {
    case MouseButton::Left: {
        return NK_BUTTON_LEFT;
    }
    case MouseButton::Right: {
        return NK_BUTTON_RIGHT;
    }
    case MouseButton::Middle: {
        return NK_BUTTON_MIDDLE;
    }
    default: {
        return static_cast<nk_buttons>(-1);
    }
    }
}

struct Engine::GuiFontData {
    Canvas2D* canvas{nullptr};
    AssetFontFacePtr fontFace{nullptr};
    struct nk_user_font data {};
};

static float getTextWidth(const nk_handle handle, const float h, const char* str, const int len) {
    auto& font = *static_cast<GuiFontData*>(handle.ptr);
    auto& canvas = *font.canvas;
    canvas.fontFace(font.fontFace->getHandle());
    canvas.fontSize(h);
    return canvas.textBounds(str, str + len).x;
}

GuiContext::GuiContext(Canvas2D& canvas, const Config& config, AssetManager& assetManager)
    : canvas(canvas), config(config) {

    fontFaceRegular = assetManager.find<AssetFontFace>(config.guiFontFaceRegular);
    fontFaceBold = assetManager.find<AssetFontFace>(config.guiFontFaceBold);
    /*userFont = std::make_unique<GuiFontData>();
    userFont->canvas = &canvas;
    userFont->data.width = getTextWidth;
    userFont->data.userdata.ptr = userFont.get();*/

    setFont(fontFaceRegular, config.guiFontSize);

    ctx = std::make_unique<nk_context>();
    nk_init_default(ctx.get(), &fonts.at(fontFaceRegular)->data);

    applyTheme();
}

GuiContext::~GuiContext() = default;

void GuiContext::setFont(const AssetFontFacePtr& fontFace, const float height) {
    auto& data = fonts[fontFace];
    if (!data) {
        data = std::make_unique<GuiFontData>();
        data->data.userdata.ptr = data.get();
        data->data.width = getTextWidth;
        data->canvas = &canvas;
    }

    data->data.height = height;
    data->fontFace = fontFace;
    if (ctx) {
        ctx->style.font = &data->data;
    }
}

void GuiContext::renderInternal(const Vector2& viewport) {
    nk_input_begin(ctx.get());
    while (!inputEvents.empty()) {
        const auto& event = inputEvents.front();
        inputEvents.pop();

        switch (event.type) {
        /*case InputEventType::KeyReleased: {
            switch (event.data.key.key) {
            case Key::LeftShift: {
                nk_input_key(ctx.get(), NK_KEY_SHIFT, event.data.key.pressed);
                break;
            }
            case Key::RightShift: {
                nk_input_key(ctx.get(), NK_KEY_SHIFT, event.data.key.pressed);
                break;
            }
            default: {
                break;
            }
            }
            break;
        }*/
        /*case InputEventType::KeyPressed: {
            switch (event.data.key.key) {
            case Key::Backspace: {
                nk_input_key(ctx.get(), NK_KEY_BACKSPACE, event.data.key.pressed);
                break;
            }
            case Key::Delete: {
                nk_input_key(ctx.get(), NK_KEY_DEL, event.data.key.pressed);
                break;
            }
            case Key::Enter: {
                nk_input_key(ctx.get(), NK_KEY_ENTER, event.data.key.pressed);
                break;
            }
            case Key::Tab: {
                nk_input_key(ctx.get(), NK_KEY_TAB, event.data.key.pressed);
                break;
            }
            case Key::LeftShift: {
                nk_input_key(ctx.get(), NK_KEY_SHIFT, event.data.key.pressed);
                break;
            }
            case Key::RightShift: {
                nk_input_key(ctx.get(), NK_KEY_SHIFT, event.data.key.pressed);
                break;
            }
            case Key::Left: {
                if (event.data.key.modifiers & Platform::KeyEvent::Modifier::Ctrl) {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_WORD_LEFT, event.data.key.pressed);
                } else {
                    nk_input_key(ctx.get(), NK_KEY_LEFT, event.data.key.pressed);
                }
                break;
            }
            case Platform::KeyEvent::Key::Right: {
                if (event.data.key.modifiers & Platform::KeyEvent::Modifier::Ctrl) {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_WORD_RIGHT, event.data.key.pressed);
                } else {
                    nk_input_key(ctx.get(), NK_KEY_RIGHT, event.data.key.pressed);
                }
                break;
            }
            case Platform::KeyEvent::Key::Home: {
                if (event.data.key.modifiers & Platform::KeyEvent::Modifier::Ctrl) {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_START, event.data.key.pressed);
                } else {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_LINE_START, event.data.key.pressed);
                }
                break;
            }
            case Platform::KeyEvent::Key::End: {
                if (event.data.key.modifiers & Platform::KeyEvent::Modifier::Ctrl) {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_END, event.data.key.pressed);
                } else {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_LINE_END, event.data.key.pressed);
                }
                break;
            }
            case Platform::KeyEvent::Key::Z: {
                if (event.data.key.modifiers & Platform::KeyEvent::Modifier::Ctrl) {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_UNDO, event.data.key.pressed);
                }
                break;
            }
            case Platform::KeyEvent::Key::Y: {
                if (event.data.key.modifiers & Platform::KeyEvent::Modifier::Ctrl) {
                    nk_input_key(ctx.get(), NK_KEY_TEXT_REDO, event.data.key.pressed);
                }
                break;
            }
            default: {
                break;
            }
            }
            break;
        }*/
        case InputEventType::MousePress: {
            const auto btn = asButton(event.data.mousePress.button);
            if (int(btn) >= 0) {
                nk_input_button(ctx.get(), btn, event.data.mousePress.x, event.data.mousePress.y, 1);
            }
            break;
        }
        case InputEventType::MouseRelease: {
            const auto btn = asButton(event.data.mouseRelease.button);
            if (int(btn) >= 0) {
                nk_input_button(ctx.get(), btn, event.data.mouseRelease.x, event.data.mouseRelease.y, 0);
            }
            break;
        }
        case InputEventType::MouseMove: {
            nk_input_motion(ctx.get(), static_cast<int>(event.data.mouseMove.x),
                            static_cast<int>(event.data.mouseMove.y));
            break;
        }
        case InputEventType::TextInput: {
            nk_input_unicode(ctx.get(), event.data.textInput.unicode);
            break;
        }
        default: {
            break;
        }
        }
    }
    nk_input_end(ctx.get());
    glEnable(GL_SCISSOR_TEST);
    const struct nk_command* cmd;
    canvas.beginPath();
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
            if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.moveTo(Vector2(c->begin.x, c->begin.y));
            canvas.strokeColor(asColor(c->color));
            canvas.strokeWidth(c->line_thickness);
            canvas.lineTo(Vector2(c->end.x, c->end.y));
            canvas.stroke();
            canvas.closePath();
            break;
        }
        case NK_COMMAND_TEXT: {
            const auto c = reinterpret_cast<const struct nk_command_text*>(cmd);
            if (c->foreground.a == 0) {
                break;
            }

            const auto fontFace = static_cast<const GuiFontData*>(c->font->userdata.ptr)->fontFace;
            canvas.fillColor(asColor(c->foreground));
            canvas.fontFace(fontFace->getHandle());
            canvas.fontSize(c->height);
            canvas.text(Vector2(c->x, static_cast<float>(c->y) + c->height / 1.25f), &c->string[0]);
            break;
        }
        case NK_COMMAND_RECT: {
            const auto c = reinterpret_cast<const struct nk_command_rect*>(cmd);
            if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.strokeColor(asColor(c->color));

            const auto p = Vector2{c->x, c->y} + Vector2{c->line_thickness / 2.0f};
            const auto s = Vector2{c->w, c->h} - Vector2{static_cast<float>(c->line_thickness)};

            canvas.roundedRect(p, s, c->rounding);
            canvas.strokeWidth(c->line_thickness);
            canvas.stroke();
            canvas.closePath();
            break;
        }
        case NK_COMMAND_RECT_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_rect_filled*>(cmd);
            if (c->color.a == 0) {
                break;
            }

            // std::cout << c->x << ", " << c->y << " " << c->w << ", " << c->h << std::endl;
            canvas.beginPath();
            canvas.fillColor(asColor(c->color));
            canvas.roundedRect(Vector2(c->x, c->y), Vector2(c->w, c->h), c->rounding);
            canvas.fill();
            canvas.closePath();
            break;
        }
        case NK_COMMAND_TRIANGLE: {
            const auto c = reinterpret_cast<const struct nk_command_triangle*>(cmd);
            if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.strokeColor(asColor(c->color));
            canvas.strokeWidth(c->line_thickness);
            canvas.moveTo(asVec(c->a));
            canvas.lineTo(asVec(c->b));
            canvas.lineTo(asVec(c->c));
            canvas.lineTo(asVec(c->a));
            canvas.stroke();
            break;
        }
        case NK_COMMAND_TRIANGLE_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_triangle_filled*>(cmd);
            if (c->color.a == 0) {
                break;
            }

            canvas.beginPath();
            canvas.fillColor(asColor(c->color));
            canvas.moveTo(asVec(c->a));
            canvas.lineTo(asVec(c->b));
            canvas.lineTo(asVec(c->c));
            canvas.lineTo(asVec(c->a));
            canvas.fill();
            break;
        }
        case NK_COMMAND_IMAGE: {
            const auto c = reinterpret_cast<const struct nk_command_image*>(cmd);
            const auto& color = asColor(c->col);
            const auto image = reinterpret_cast<Canvas2D::Image*>(c->img.handle.ptr);
            canvas.beginPath();
            canvas.rectImage({static_cast<float>(c->x), static_cast<float>(c->y)},
                             {static_cast<float>(c->w), static_cast<float>(c->h)}, *image, color);
            canvas.fill();
            canvas.closePath();
            break;
        }
        default: {
            break;
        }
        }
    }

    if (dragAndDrop.show && dragAndDrop.image) {
        canvas.beginPath();

        canvas.fillColor(asColor(ctx->style.window.background));
        canvas.strokeColor(asColor(ctx->style.window.border_color));
        canvas.strokeWidth(ctx->style.window.border);
        canvas.rect(getMousePos(), dragAndDrop.size + GuiTheme::padding * 2.0f);
        canvas.fill();
        canvas.stroke();

        canvas.fillColor(Color4{1.0f});
        canvas.rectImage(getMousePos(), dragAndDrop.size + GuiTheme::padding, dragAndDrop.image->getImage(),
                         Color4{1.0f});
        canvas.fill();

        canvas.closePath();
    }

    canvas.closePath();
    nk_clear(ctx.get());
    glDisable(GL_SCISSOR_TEST);
}

void GuiContext::reset() {
    windowsBounds.clear();
}

void GuiContext::render(const Vector2& viewport) {
    this->viewport = viewport;
    renderInternal(this->viewport);
    if (dragAndDrop.drop) {
        dragAndDrop.drop = false;
        dragAndDrop.data.reset();
    }
}

void GuiContext::spacing() {
    nk_spacing(ctx.get(), 1);
}

void GuiContext::window(const Vector2& pos, const Vector2& size, const std::string& name, const GuiFlags flags,
                        const std::function<void()>& fn) {
    if (nk_begin_titled(ctx.get(), name.c_str(), name.c_str(), nk_rect(pos.x, pos.y, size.x, size.y), flags)) {
        windowsBounds.push_back({pos, size});
        nk_window_set_position(ctx.get(), name.c_str(), nk_vec2(pos.x, pos.y));
        fn();

        nk_end(ctx.get());
    }
}

void GuiContext::group(const std::string& name, const GuiFlags flags, const std::function<void()>& fn) {
    if (nk_group_begin(ctx.get(), name.c_str(), flags)) {
        // nk_layout_row_dynamic(ctx.get(), 0.0f, 1);
        fn();
        nk_group_end(ctx.get());
    }
}

bool GuiContext::button(const std::string& text) {
    return nk_button_label(ctx.get(), text.c_str()) == nk_true;
}

void GuiContext::onNextDropOff(const std::function<void(const std::any&)>& fn) {
    if (isNextHover() && dragAndDrop.drop) {
        dragAndDrop.drop = false;
        if (fn) {
            fn(dragAndDrop.data);
        }
        dragAndDrop.data.reset();
    }
}

void GuiContext::setDragAndDrop(const AssetImagePtr& image, const std::any& data) {
    dragAndDrop.data = data;
    dragAndDrop.image = image;
    dragAndDrop.size = getWidgetSize();
}

/*bool GuiContext::draggable(const AssetImagePtr& image, const std::any& data,
                           const std::function<void(const std::any&)>& fn) {
    if (isNextHover() && dragAndDrop.drop) {
        dragAndDrop.drop = false;
        if (fn) {
            fn(dragAndDrop.data);
        }
        dragAndDrop.data.reset();
    }

    if (buttonImage(image)) {
        // Drag and drop start
        dragAndDrop.data = data;
        dragAndDrop.image = image;
        dragAndDrop.size = getWidgetSize();
        return true;
    }

    return false;
}*/

bool GuiContext::buttonImage(const AssetImagePtr& image) {
    struct nk_image img {};
    img.handle.ptr = const_cast<Canvas2D::Image*>(&image.get()->getImage());
    img.w = image->getImage().size.x;
    img.h = image->getImage().size.y;
    return nk_button_image(ctx.get(), img);
}

bool GuiContext::buttonImage(const AssetImagePtr& image, const std::string& text, const TextAlignValue align) {
    struct nk_image img {};
    img.handle.ptr = const_cast<Canvas2D::Image*>(&image.get()->getImage());
    img.w = image->getImage().size.x;
    img.h = image->getImage().size.y;
    return nk_button_image_label(ctx.get(), img, text.c_str(), align);
}

void GuiContext::label(const std::string& text, const TextAlignValue align) {
    nk_label(ctx.get(), text.c_str(), align);
}

void GuiContext::label(const std::string& text, const Color4& color, const TextAlignValue align) {
    nk_label_colored(ctx.get(), text.c_str(), align, toNkColor(color));
}

void GuiContext::selectableLabel(const std::string& text, bool& value, const TextAlignValue align) {
    nk_bool v = value;
    nk_selectable_label(ctx.get(), text.c_str(), align, &v);
    value = v;
}

void GuiContext::image(const AssetImagePtr& image) {
    struct nk_image img {};
    img.handle.ptr = const_cast<Canvas2D::Image*>(&image.get()->getImage());
    img.w = image->getImage().size.x;
    img.h = image->getImage().size.y;
    nk_image(ctx.get(), img);
}

void GuiContext::title(const std::string& text) {
    setFont(fontFaceBold, config.guiFontSize + 2.0f);
    label(text);
    setFont(fontFaceRegular, config.guiFontSize);
}

void GuiContext::text(const std::string& text) {
    nk_text_wrap(ctx.get(), text.c_str(), static_cast<int>(text.size()));
}

void GuiContext::layoutDynamic(const float height, const int count) {
    nk_layout_row_dynamic(ctx.get(), height, count);
}

void GuiContext::layoutDynamic(const float height, const int count, const std::function<void()>& fn) {
    nk_layout_row_begin(ctx.get(), NK_DYNAMIC, height, count);
    fn();
    nk_layout_row_end(ctx.get());
}

void GuiContext::layoutDynamicPush(const float weight) {
    nk_layout_row_push(ctx.get(), weight);
}

void GuiContext::layoutTemplated(const float height, const std::vector<GuiRowTemplate>& columns) {
    nk_layout_row_template_begin(ctx.get(), height);
    for (const auto& column : columns) {
        switch (column.type) {
        case GuiRowTemplateType::Dynamic: {
            nk_layout_row_template_push_dynamic(ctx.get());
            break;
        }
        case GuiRowTemplateType::Static: {
            nk_layout_row_template_push_static(ctx.get(), column.value);
            break;
        }
        case GuiRowTemplateType::Variable: {
            nk_layout_row_template_push_variable(ctx.get(), column.value);
            break;
        }
        default: {
            break;
        }
        }
    }
    nk_layout_row_template_end(ctx.get());
}

void GuiContext::combo(const std::string& selected, const Vector2& size, const std::function<void()>& fn) {
    if (nk_combo_begin_label(ctx.get(), selected.c_str(), nk_vec2(size.x, size.y))) {
        fn();
        nk_combo_end(ctx.get());
    }
}

bool GuiContext::comboItem(const std::string& text) {
    const auto res = nk_combo_item_label(ctx.get(), text.c_str(), NK_TEXT_LEFT) == nk_true;
    if (res) {
        ctx->input.mouse.pos = nk_vec2(0, 0);
        ctx->input.mouse.buttons[NK_BUTTON_LEFT].down = 0;
    }
    return res;
}

void GuiContext::tooltip(const std::string& text) {
    if (isNextHover()) {
        nk_tooltip(ctx.get(), text.c_str());
    }
}

void GuiContext::tooltip(const float width, const std::function<void()>& fn) {
    if (isNextHover()) {
        if (nk_tooltip_begin(ctx.get(), width)) {
            fn();
            nk_tooltip_end(ctx.get());
        }
    }
}

void GuiContext::input(std::string& text, const size_t max) {
    std::vector<char> buff;
    buff.resize(max);
    std::memcpy(buff.data(), text.data(), std::min(text.size(), max));

    int len = static_cast<int>(text.size());
    nk_edit_string(ctx.get(), NK_EDIT_FIELD, buff.data(), &len, static_cast<int>(max), nk_filter_default);

    if (len != text.size()) {
        text.resize(len);
    }

    std::memcpy(text.data(), buff.data(), len);
}

void GuiContext::progress(float progress) {
    auto current = static_cast<nk_size>(progress * 100.0f);
    nk_progress(ctx.get(), &current, 100, nk_false);
}

bool GuiContext::isNextHover() const {
    const struct nk_input* in = &ctx->input;
    const auto bounds = nk_widget_bounds(ctx.get());
    return nk_input_is_mouse_hovering_rect(in, bounds) == nk_true;
}

Vector2 GuiContext::getContentSize() const {
    const auto bounds = nk_layout_widget_bounds(ctx.get());
    const auto content = nk_window_get_content_region_size(ctx.get());
    return {content.x, content.y - bounds.h};
}

Vector2 GuiContext::getWidgetSize() const {
    const auto size = nk_widget_size(ctx.get());
    return {size.x, size.y};
}

Vector2 GuiContext::getMousePos() const {
    const auto& vec = ctx->input.mouse.pos;
    return {vec.x, vec.y};
}

void GuiContext::keyPressEvent(const Engine::Key& key) {
    InputEventData data{};
    data.key.key = key;
    data.key.pressed = true;
    inputEvents.push({data, InputEventType::KeyPressed});
}

void GuiContext::keyReleaseEvent(const Engine::Key& key) {
    InputEventData data{};
    data.key.key = key;
    data.key.pressed = false;
    inputEvents.push({data, InputEventType::KeyReleased});
}

void GuiContext::mousePressEvent(const Vector2i& pos, const MouseButton button) {
    InputEventData data{};
    data.mousePress.button = button;
    data.mousePress.x = pos.x;
    data.mousePress.y = pos.y;
    inputEvents.push({data, InputEventType::MousePress});
}

void GuiContext::mouseReleaseEvent(const Vector2i& pos, const MouseButton button) {
    InputEventData data{};
    data.mouseRelease.button = button;
    data.mouseRelease.x = pos.x;
    data.mouseRelease.y = pos.y;
    inputEvents.push({data, InputEventType::MouseRelease});

    if (dragAndDrop.data.has_value() || dragAndDrop.show) {
        dragAndDrop.show = false;
        dragAndDrop.drop = true;
    }
}

void GuiContext::mouseMoveEvent(const Vector2i& pos) {
    InputEventData data{};
    data.mouseMove.x = pos.x;
    data.mouseMove.y = pos.y;
    inputEvents.push({data, InputEventType::MouseMove});

    if (dragAndDrop.data.has_value() && !dragAndDrop.show) {
        dragAndDrop.show = true;
        dragAndDrop.drop = false;
    }
}

void GuiContext::textInputEvent(const int c) {
    InputEventData data{};

    /*const auto u = Corrade::Utility::Unicode::utf32(std::string(event.text().data(), event.text().size()));
    data.textInput.unicode = u.front();
    inputEvents.push({data, InputEventType::TextInput});*/
}

bool GuiContext::inputOverlap(const Vector2i& pos) const {
    for (const auto& bounds : windowsBounds) {
        if (pos.x >= bounds.pos.x && pos.x <= bounds.pos.x + bounds.size.x && pos.y >= bounds.pos.y &&
            pos.y <= bounds.pos.y + bounds.size.y) {
            return true;
        }
    }
    return false;
}

void GuiContext::applyTheme() {
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
    window.header.padding = nk_vec2(3, 2);
    window.group_padding = nk_vec2(0, 0);
    window.padding = nk_vec2(GuiTheme::padding, GuiTheme::padding);
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
