#include "Nuklear.hpp"
// #define NK_IMPLEMENTATION 1
// #define NK_INCLUDE_DEFAULT_ALLOCATOR 1
// #include <nuklear.h>

using namespace Engine;

/*static Color4 asColor(const nk_color& color) {
    return {static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f,
            static_cast<float>(color.a) / 255.0f};
}

static float getTextWidth(const nk_handle handle, const float height, const char* str, const int len) {
    auto& font = *static_cast<const FontFace*>(handle.ptr);
    return font.getBounds({str, static_cast<size_t>(len)}, height).x;
}

static nk_color HEX(const uint32_t v) {
    return nk_rgba((v & 0xFF000000) >> 24, (v & 0x00FF0000) >> 16, (v & 0x0000FF00) >> 8, (v & 0x000000FF) >> 0);
}

static nk_color fromColor(const Color4& color) {
    return nk_rgba(color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

struct Nuklear::CustomStyle {
    nk_style_button image{};
    nk_style_button toggle{};
};*/

Nuklear::Nuklear(EventCallback& events, const Config& config, const FontFamily& defaultFontFamily,
                 const int defaultFontSize) /* :
     events{events},
     config{config},
     canvas{canvas},
     defaultFontFamily{defaultFontFamily},
     defaultFontSize{defaultFontSize},
     customStyle{std::make_unique<CustomStyle>()},
     ctx{std::make_unique<nk_context>()},
     activeInput{false}*/
{

    /*defaultFont = &addFontFamily(defaultFontFamily, defaultFontSize);
    nk_init_default(ctx.get(), defaultFont);*/

    applyTheme();
}

Nuklear::~Nuklear() {
    // nk_free(ctx.get());
}

void Nuklear::fontSize(const int size) {
    // nk_style_set_font(ctx.get(), &addFontFamily(defaultFontFamily, size));
}

void Nuklear::resetFont() {
    // nk_style_set_font(ctx.get(), defaultFont);
}

/*nk_user_font& Nuklear::addFontFamily(const FontFamily& fontFamily, int size) {
    auto& fontSizes = fonts[&fontFamily];
    auto it = fontSizes.find(size);

    if (it == fontSizes.end()) {
        it = fontSizes.insert(std::make_pair(size, std::make_unique<nk_user_font>())).first;

        it->second->height = static_cast<float>(size);
        it->second->width = &getTextWidth;
        it->second->userdata.ptr = const_cast<void*>(reinterpret_cast<const void*>(&fontFamily));
    }

    return *it->second;
}*/

void Nuklear::begin(const Vector2i& viewport) {
    lastViewportValue = viewport;
    windowsBounds.clear();
    activeInput = false;
    resetFont();
    inputPoll();
}

void Nuklear::end() {
    if (dragAndDrop.value.has_value() && dragAndDrop.image) {
        if (!inputHasMouseDown()) {
            dragAndDrop.value.reset();
        } else {
            drawDragAndDrop();
        }
    }

    render();
    // nk_clear(ctx.get());
}

void Engine::Nuklear::drawDragAndDrop() {
    /*const auto pad = Vector2{ctx->style.window.padding.x, ctx->style.window.padding.y} * 2.0f;
    const auto size = Vector2{config.gui.dragAndDropSize} + pad;
    const auto flags = WindowFlags::NoInput | WindowFlags::NoScrollbar | WindowFlags::Border;
    if (beginWindow("Drag And Drop", getMousePos(), size, flags)) {
        layoutDynamic(config.gui.dragAndDropSize, 1);
        image(dragAndDrop.image);
    }
    endWindow();*/
}

void Nuklear::inputPoll() {
    /*nk_input_begin(ctx.get());
    for (const auto& event : inputEvents) {
        event();
    }
    inputEvents.clear();
    nk_input_end(ctx.get());*/
}

void Nuklear::render() {
    /*const struct nk_command* cmd;
    nk_foreach(cmd, ctx.get()) {
        switch (cmd->type) {
        case NK_COMMAND_SCISSOR: {
            const auto c = reinterpret_cast<const struct nk_command_scissor*>(cmd);
            if (c->x >= 0 && c->y >= 0) {
                canvas.scissor({static_cast<float>(c->x), static_cast<float>(c->y)},
                               {static_cast<float>(c->w), static_cast<float>(c->h)});
            } else {
                canvas.scissor({0, 0}, lastViewportValue);
            }

            break;
        }
        case NK_COMMAND_LINE: {
            const auto c = reinterpret_cast<const struct nk_command_line*>(cmd);
            break;
        }
        case NK_COMMAND_TEXT: {
            const auto c = reinterpret_cast<const struct nk_command_text*>(cmd);
            auto& font = *static_cast<const FontFamily*>(c->font->userdata.ptr);
            canvas.color(asColor(c->foreground));
            canvas.font(font.get(FontFace::Regular), c->font->height);
            std::string_view text{&c->string[0], std::strlen(&c->string[0])};
            canvas.text(Vector2(c->x, static_cast<float>(c->y) + c->height / 1.25f), text);
            break;
        }
        case NK_COMMAND_RECT: {
            const auto c = reinterpret_cast<const struct nk_command_rect*>(cmd);
            const auto p = Vector2{c->x, c->y};
            const auto s = Vector2{c->w, c->h};
            canvas.color(asColor(c->color));
            canvas.rectOutline(p, s, static_cast<float>(c->line_thickness));
            break;
        }
        case NK_COMMAND_RECT_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_rect_filled*>(cmd);
            canvas.color(asColor(c->color));
            canvas.rect(Vector2(c->x, c->y), Vector2(c->w, c->h));
            break;
        }
        case NK_COMMAND_TRIANGLE: {
            const auto c = reinterpret_cast<const struct nk_command_triangle*>(cmd);
            break;
        }
        case NK_COMMAND_TRIANGLE_FILLED: {
            const auto c = reinterpret_cast<const struct nk_command_triangle_filled*>(cmd);
            break;
        }
        case NK_COMMAND_IMAGE: {
            const auto c = reinterpret_cast<const struct nk_command_image*>(cmd);
            const auto& color = asColor(c->col);
            const auto image = reinterpret_cast<Image*>(c->img.handle.ptr);
            if (image) {
                canvas.color(color);
                canvas.image({static_cast<float>(c->x), static_cast<float>(c->y)},
                             {static_cast<float>(c->w), static_cast<float>(c->h)},
                             *image);
            }
            break;
        }
        default: {
            break;
        }
        }
    }*/
}

bool Nuklear::beginWindow(const std::string& title, const Vector2& pos, const Vector2& size, const Flags flags) {
    /*if (nk_begin_titled(ctx.get(), title.c_str(), title.c_str(), nk_rect(pos.x, pos.y, size.x, size.y), flags)) {
        windowsBounds.emplace_back(pos, size);
        if (!(flags & static_cast<Flags>(Nuklear::WindowFlags::Moveable))) {
            nk_window_set_position(ctx.get(), title.c_str(), nk_vec2(pos.x, pos.y));
        }
        return true;
    }*/

    return false;
}

void Nuklear::endWindow() {
    // nk_end(ctx.get());
}

void Nuklear::layoutDynamic(const float height, const int count) {
    // nk_layout_row_dynamic(ctx.get(), height, count);
}

void Nuklear::layoutStatic(const float height, const float width, const int count) {
    // nk_layout_row_static(ctx.get(), height, width, count);
}

void Nuklear::layoutSkip() {
    // struct nk_rect bounds {};
    // nk_widget(&bounds, ctx.get());
}

void Engine::Nuklear::layoutTemplateBegin(const float height) {
    // nk_layout_row_template_begin(ctx.get(), height);
}

void Engine::Nuklear::layoutTemplateDynamic() {
    // nk_layout_row_template_push_dynamic(ctx.get());
}

void Engine::Nuklear::layoutTemplateVariable(const float value) {
    // nk_layout_row_template_push_variable(ctx.get(), value);
}

void Engine::Nuklear::layoutTemplateStatic(const float value) {
    // nk_layout_row_template_push_static(ctx.get(), value);
}

void Engine::Nuklear::layoutTemplateEnd() {
    // nk_layout_row_template_end(ctx.get());
}

void Nuklear::layoutBeginDynamic(const float height, const int count) {
    // nk_layout_row_begin(ctx.get(), NK_DYNAMIC, height, count);
}

void Nuklear::layoutEnd() {
    // nk_layout_row_end(ctx.get());
}

void Nuklear::layoutPush(float value) {
    // nk_layout_row_push(ctx.get(), value);
}

bool Nuklear::groupBegin(const std::string& name, bool scrollbar) {
    /*Flags flags = 0;
    if (!scrollbar) {
        flags = flags | WindowFlags::NoScrollbar;
    }

    return nk_group_begin(ctx.get(), name.c_str(), flags) == nk_true;*/
    return false;
}

void Nuklear::groupEnd() {
    // nk_group_end(ctx.get());
}

bool Engine::Nuklear::isHovered() {
    // return nk_widget_is_hovered(ctx.get()) == nk_true;
    /*const auto bounds = nk_widget_bounds(ctx.get());
    return nk_input_is_mouse_hovering_rect(&ctx->input, bounds) == nk_true;*/
    return false;
}

bool Engine::Nuklear::isClicked(MouseButton button) {
    /*nk_buttons nb;
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

    const auto bounds = nk_widget_bounds(ctx.get());
    return nk_input_is_mouse_click_in_rect(&ctx->input, nb, bounds) == nk_true;*/
    return false;
}

bool Engine::Nuklear::isMouseDown(MouseButton button) {
    /*nk_buttons nb;
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

    const auto bounds = nk_widget_bounds(ctx.get());
    return nk_input_has_mouse_click_down_in_rect(&ctx->input, nb, bounds, nk_true) == nk_true;*/
    return false;
}

void Engine::Nuklear::triggerEventClick() {
    // events.nuklearOnClick(true);
}

bool Nuklear::button(const std::string& text, const TextAlign align) {
    /*nk_style_button& style = ctx->style.button;
    style.text_alignment = static_cast<nk_flags>(align);

    if (nk_button_label(ctx.get(), text.c_str()) == nk_true) {
        triggerEventClick();
        return true;
    }*/
    return false;
}

void Nuklear::buttonToggle(const std::string& text, bool& value, Nuklear::TextAlign align) {
    /*if (nk_widget_is_mouse_clicked(ctx.get(), nk_buttons::NK_BUTTON_LEFT)) {
        triggerEventClick();
    }

    setStyleButtonToggle(value);

    if (nk_button_label_styled(ctx.get(), &customStyle->toggle, text.c_str())) {
        value = true;
    }*/
}

bool Nuklear::buttonImage(const ImagePtr& img) {
    /*struct nk_image ni {};
    ni.handle.ptr = img.get();

    if (nk_button_image(ctx.get(), ni) == nk_true) {
        triggerEventClick();
        return true;
    }*/
    return false;
}

void Engine::Nuklear::buttonImageToggle(const ImagePtr& img, bool& value) {
    /*struct nk_image ni {};
    ni.handle.ptr = img.get();

    setStyleImageToggle(value);

    if (nk_button_image_styled(ctx.get(), &customStyle->image, ni) == nk_true) {
        triggerEventClick();
        value = true;
    }*/
}

bool Engine::Nuklear::button(const Color4& color) {
    /*const auto nc = fromColor(color);

    if (nk_button_color(ctx.get(), nc) == nk_true) {
        triggerEventClick();
        return true;
    }*/
    return false;
}

void Engine::Nuklear::selectable(const std::string& text, bool& value) {
    /*nk_bool val = value ? nk_true : nk_false;

    if (nk_selectable_label(ctx.get(), text.c_str(), NK_TEXT_ALIGN_LEFT + NK_TEXT_ALIGN_MIDDLE, &val)) {
        triggerEventClick();
        value = true;
    }*/
}

bool Nuklear::image(const ImagePtr& img) {
    /*struct nk_image ni {};
    ni.handle.ptr = img.get();

    if (isHovered()) {
        if (isMouseDown()) {
            customStyle->image.border_color = ctx->style.button.active.data.color;
        } else {
            customStyle->image.border_color = ctx->style.button.hover.data.color;
        }
    } else {
        customStyle->image.border_color = ctx->style.button.border_color;
    }

    customStyle->image.text_background.a = 0.0f;

    if (nk_button_image_styled(ctx.get(), &customStyle->image, ni) == nk_true) {
        triggerEventClick();
        return true;
    }*/
    return false;
}

void Engine::Nuklear::setStyleImageToggle(const bool value) {
    /*if (value) {
        if (isHovered()) {
            customStyle->image.border_color = ctx->style.button.hover.data.color;
        } else {
            customStyle->image.border_color = ctx->style.button.active.data.color;
        }
    } else {
        if (isHovered()) {
            customStyle->image.border_color = ctx->style.button.hover.data.color;
        } else {
            customStyle->image.border_color = ctx->style.button.border_color;
        }
    }*/
}

void Engine::Nuklear::setStyleButtonToggle(const bool value) {
    /*if (value) {
        if (isHovered()) {
            customStyle->toggle.border_color = ctx->style.button.hover.data.color;
        } else {
            customStyle->toggle.border_color = ctx->style.button.active.data.color;
        }
    } else {
        if (isHovered()) {
            customStyle->toggle.border_color = ctx->style.button.hover.data.color;
        } else {
            customStyle->toggle.border_color = ctx->style.button.border_color;
        }
    }*/
}

void Nuklear::imageToggle(const ImagePtr& img, bool& value) {
    /*struct nk_image ni {};
    ni.handle.ptr = img.get();

    setStyleImageToggle(value);

    if (nk_button_image_styled(ctx.get(), &customStyle->image, ni) == nk_true) {
        triggerEventClick();
        value = true;
    }*/
}

void Nuklear::imageToggle(const ImagePtr& img, bool& value, const std::string& text, TextAlign align) {
    /*struct nk_image ni {};
    ni.handle.ptr = img.get();

    setStyleImageToggle(value);

    const auto flags = static_cast<nk_flags>(align);
    if (nk_button_image_label_styled(ctx.get(), &customStyle->image, ni, text.c_str(), flags) == nk_true) {
        triggerEventClick();
        value = true;
    }*/
}

void Nuklear::label(const std::string& text, TextAlign align) {
    // nk_label(ctx.get(), text.c_str(), static_cast<nk_flags>(align));
}

void Nuklear::text(const std::string& text) {
    // nk_label_wrap(ctx.get(), text.c_str());
}

void Nuklear::input(std::string& text, const size_t max) {
    /*if (editBuffer.size() < max + 1) {
        editBuffer.resize(max + 1);
    }
    std::memcpy(editBuffer.data(), text.data(), std::min(text.size(), max));
    editBuffer[text.size()] = '\0';

    auto len = static_cast<int>(std::min(text.size(), max));

    const auto state = nk_edit_string(ctx.get(), NK_EDIT_SIMPLE, editBuffer.data(), &len, max, nk_filter_default);
    if (len != text.size()) {
        text.resize(len);
        std::memcpy(text.data(), editBuffer.data(), text.size());
    }

    activeInput = !(state & NK_WIDGET_STATE_ACTIVE);*/
}

void Nuklear::progress(float value) {
    /*auto current = static_cast<nk_size>(value * 100.0f);
    nk_progress(ctx.get(), &current, 100, nk_false);*/
}

void Engine::Nuklear::tooltip(const std::string& text) {
    /*if (isHovered()) {
        nk_tooltip(ctx.get(), text.c_str());
    }*/
}

void Engine::Nuklear::checkbox(const std::string& text, bool& value) {
    /*if (nk_widget_is_mouse_clicked(ctx.get(), nk_buttons::NK_BUTTON_LEFT)) {
        triggerEventClick();
    }

    value = nk_check_label(ctx.get(), text.c_str(), value ? nk_true : nk_false) == nk_true;*/
}

bool Engine::Nuklear::comboBegin(const Color4& color, const Vector2& size) {
    /*const auto nc = fromColor(color);
    struct nk_vec2 s {
        size.x, size.y,
    };

    return nk_combo_begin_color(ctx.get(), nc, s) == nk_true;*/
    return false;
}

void Engine::Nuklear::combo(const Vector2& size, size_t& choice, const std::vector<std::string>& items) {
    /*struct nk_vec2 s {
        size.x, size.y,
    };
    const auto height = nk_widget_height(ctx.get());
    auto* c = items.at(choice < items.size() ? choice : 0).data();

    if (nk_combo_begin_label(ctx.get(), c, s)) {
        for (size_t i = 0; i < items.size(); i++) {
            nk_layout_row_dynamic(ctx.get(), height, 1);

            if (nk_combo_item_label(ctx.get(), items.at(i).c_str(), NK_TEXT_ALIGN_LEFT)) {
                triggerEventClick();
                choice = i;
            }
        }
        nk_combo_end(ctx.get());
    }*/
}

void Engine::Nuklear::comboEnd() {
    // nk_combo_end(ctx.get());
}

void Engine::Nuklear::comboClose() {
    // nk_combo_close(ctx.get());
}

bool Engine::Nuklear::popupBegin(const std::string& name, const Vector2& pos, const Vector2& size) {
    /*struct nk_rect rect {
        pos.x, pos.y, size.x, size.y
    };
    const auto flags = WindowFlags::Border | WindowFlags::NoScrollbar;
    return nk_popup_begin(ctx.get(), NK_POPUP_STATIC, name.c_str(), NK_WINDOW_CLOSABLE, rect) == nk_true;*/
    return false;
}

void Engine::Nuklear::popupEnd() {
    // nk_popup_end(ctx.get());
}

void Engine::Nuklear::popupClose() {
    // nk_popup_close(ctx.get());
}

void Nuklear::eventMouseMoved(const Vector2i& pos) {
    // inputEvents.emplace_back([=]() { nk_input_motion(ctx.get(), pos.x, pos.y); });
}

/*static nk_buttons toNkButtons(const MouseButton button) {
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
}*/

/*static nk_keys toNkKeys(const Key key) {
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
}*/

void Nuklear::eventMousePressed(const Vector2i& pos, MouseButton button) {
    // inputEvents.emplace_back([=]() { nk_input_button(ctx.get(), toNkButtons(button), pos.x, pos.y, true); });
}

void Nuklear::eventMouseReleased(const Vector2i& pos, MouseButton button) {
    // inputEvents.emplace_back([=]() { nk_input_button(ctx.get(), toNkButtons(button), pos.x, pos.y, false); });
}

void Nuklear::eventMouseScroll(int xscroll, int yscroll) {
    // inputEvents.emplace_back([=]() { nk_input_scroll(ctx.get(), nk_vec2(xscroll, yscroll)); });
}

void Nuklear::eventKeyPressed(Key key, Modifiers modifiers) {
    /*const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        inputEvents.emplace_back([=]() { nk_input_key(ctx.get(), toNkKeys(key), true); });
    }*/
}

void Nuklear::eventKeyReleased(Key key, Modifiers modifiers) {
    /*const auto k = toNkKeys(key);
    if (k != nk_keys::NK_KEY_NONE) {
        inputEvents.emplace_back([=]() { nk_input_key(ctx.get(), toNkKeys(key), false); });
    }*/
}

void Nuklear::eventCharTyped(uint32_t code) {
    // inputEvents.emplace_back([=]() { nk_input_unicode(ctx.get(), code); });
}

bool Nuklear::isCursorInsideWindow(const Vector2i& mousePos) const {
    /*for (const auto& [pos, size] : windowsBounds) {
        if (mousePos.x > pos.x && mousePos.x < pos.x + size.x && mousePos.y > pos.y && mousePos.y < pos.y + size.y) {
            return true;
        }
    }*/
    return false;
}

Vector2 Nuklear::getContentRegion() const {
    // const auto space = nk_window_get_content_region(ctx.get());
    // return Vector2{space.w, space.h} - Vector2{ctx->style.window.padding.x, ctx->style.window.padding.y} * 2.0f;
    return {0.0f, 0.0f};
}

Vector2 Nuklear::getContentPos() const {
    /*const auto space = nk_window_get_content_region(ctx.get());
    return Vector2{space.x, space.y};*/
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getMousePos() const {
    // return {ctx->input.mouse.pos.x, ctx->input.mouse.pos.y};
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getWindowSizeForContentRegion(const Vector2& size) const {
    // return size + Vector2{ctx->style.window.padding.x, ctx->style.window.padding.y} * 2.0f;
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getSpacing() const {
    // return Vector2{ctx->style.window.spacing.x, ctx->style.window.spacing.y};
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getPadding() const {
    // return Vector2{ctx->style.window.padding.x, ctx->style.window.padding.y};
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getGroupPadding() const {
    // return Vector2{ctx->style.window.group_padding.x, ctx->style.window.group_padding.y};
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getWindowPos() const {
    /*if (!ctx->active) {
        EXCEPTION("No active gui window");
    }
    return {ctx->active->bounds.x, ctx->active->bounds.y};*/
    return {0.0f, 0.0f};
}

Vector2 Engine::Nuklear::getWidgetSize() const {
    /*return {
        nk_widget_width(ctx.get()),
        nk_widget_height(ctx.get()),
    };*/
    return {0.0f, 0.0f};
}

bool Engine::Nuklear::inputHasMouseDown(MouseButton button) {
    /*nk_buttons nb;
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
    return ctx->input.mouse.buttons[nb].down == nk_true;*/
    return false;
}

/* const auto BLACK = HEX(0x000000ff);
static const auto PRIMARY_COLOR = fromColor(Theme::primary);
static const auto WHITE = HEX(0xe5e5e3ff);
static const auto TEXT_WHITE = fromColor(Theme::text);
static const auto TEXT_BLACK = HEX(0x030303ff);
static const auto TEXT_GREY = HEX(0x393939ff);
static const auto BACKGROUND_COLOR = fromColor(Theme::backgroundTransparent); // HEX(0x0a0a0aff);
static const auto BORDER_GREY = HEX(0x202020ff);
static const auto TRANSPARENT_COLOR = HEX(0x00000000);
static const auto ACTIVE_COLOR = PRIMARY_COLOR;*/

void Nuklear::applyTheme() {
    /*auto& window = ctx->style.window;
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

    customStyle->image = button;
    customStyle->image.normal.data.color = TRANSPARENT_COLOR;
    customStyle->image.hover.data.color = TRANSPARENT_COLOR;
    customStyle->image.active.data.color = TRANSPARENT_COLOR;

    customStyle->toggle = button;*/
}
