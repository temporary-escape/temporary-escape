#define NANOVG_GL3_IMPLEMENTATION
#include "Canvas2D.hpp"
#include "../Utils/Exceptions.hpp"
#include <nanovg.h>
#include <nanovg_gl.h>

using namespace Scissio;

static NVGcolor toNvg(const Color4& color) {
    // return *reinterpret_cast<const NVGcolor*>(&color.r);
    NVGcolor nc;
    nc.r = color.r;
    nc.g = color.g;
    nc.b = color.b;
    nc.a = color.a;
    return nc;
}

Canvas2D::Canvas2D() {
    vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
}

Canvas2D::Canvas2D(const NoCreate&) : vg(nullptr) {
}

Canvas2D::~Canvas2D() {
    if (vg) {
        fontCache.clear();
        nvgDeleteGL3(vg);
        vg = nullptr;
    }
}

void Canvas2D::beginFrame(const Vector2i& viewport) {
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVao);
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    nvgBeginFrame(vg, static_cast<float>(viewport.x), static_cast<float>(viewport.y), 1.0f);
}

void Canvas2D::endFrame() {
    nvgEndFrame(vg);
    glBindVertexArray(currentVao);
    glUseProgram(currentProgram);
}

void Canvas2D::scissor(const Vector2& pos, const Vector2& size) const {
    nvgScissor(vg, pos.x, pos.y, size.x, size.y);
}

void Canvas2D::beginPath() const {
    nvgBeginPath(vg);
}

void Canvas2D::closePath() const {
    nvgClosePath(vg);
}

void Canvas2D::rect(const Vector2& pos, const Vector2& size) const {
    nvgRect(vg, pos.x, pos.y, size.x, size.y);
}

void Canvas2D::circle(const Vector2& pos, const float radius) const {
    nvgCircle(vg, pos.x, pos.y, radius);
}

void Canvas2D::roundedRect(const Vector2& pos, const Vector2& size, const float radius) const {
    nvgRoundedRect(vg, pos.x, pos.y, size.x, size.y, radius);
}

void Canvas2D::moveTo(const Vector2& pos) const {
    nvgMoveTo(vg, pos.x, pos.y);
}

void Canvas2D::lineTo(const Vector2& pos) const {
    nvgLineTo(vg, pos.x, pos.y);
}

void Canvas2D::fill() const {
    nvgFill(vg);
}

void Canvas2D::stroke() const {
    nvgStroke(vg);
}

void Canvas2D::fillColor(const Color4& color) const {
    nvgFillColor(vg, toNvg(color));
}

void Canvas2D::strokeColor(const Color4& color) const {
    nvgStrokeColor(vg, toNvg(color));
}

void Canvas2D::strokeWidth(const float width) const {
    nvgStrokeWidth(vg, width);
}

Canvas2D::FontHandle Canvas2D::loadFont(const Path& path) {
    const auto pathStr = path.string();
    auto it = fontCache.find(pathStr);
    if (it == fontCache.end()) {
        const auto handle = nvgCreateFont(vg, pathStr.c_str(), pathStr.c_str());
        if (handle < 0)
            EXCEPTION("Failed to load Canvas2D font from: {}", path.string());
        it = fontCache.insert(std::make_pair(pathStr, handle)).first;
    }
    return it->second;
}

Canvas2D::ImageHandle Canvas2D::textureToImageHandle(const Texture2D& texture, const Vector2i& size) {
    return nvglCreateImageFromHandleGL3(vg, texture.getHandle(), size.x, size.y, NVGcreateFlags::NVG_ANTIALIAS);
}

void Canvas2D::rectImage(const Vector2& pos, const Vector2& size, const Image& image, const Color4& color) {
    const auto& imageSize = Vector2{image.size};
    const auto& scale = size / imageSize;
    const auto& atlasSize = Vector2{image.atlasSize} * scale;
    const auto& imagePos = Vector2{image.pos} * scale;
    auto pattern = nvgImagePattern(vg, pos.x - imagePos.x, pos.y + imagePos.y + (imageSize.y * scale.y), atlasSize.x,
                                   -atlasSize.y, 0.0f, image.handle, 1.0f);
    pattern.innerColor = toNvg(color);
    nvgFillPaint(vg, pattern);
    rect(pos, size);
}

void Canvas2D::fontFace(const FontHandle& font) const {
    if (font < 0)
        EXCEPTION("Bad font handle");

    nvgFontFaceId(vg, font);
}

void Canvas2D::fontSize(const float size) const {
    nvgFontSize(vg, size);
}

void Canvas2D::text(const Vector2& pos, const std::string& str) const {
    nvgText(vg, pos.x, pos.y, str.c_str(), str.c_str() + str.size());
}

void Canvas2D::textBox(const Vector2& pos, const float width, const std::string& str) const {
    nvgTextBox(vg, pos.x, pos.y, width, str.c_str(), str.c_str() + str.size());
}

Vector2 Canvas2D::textBounds(const std::string::value_type* start, const std::string::value_type* end) const {
    float bounds[4];
    nvgTextBounds(vg, 0.0f, 0.0f, start, end, bounds);
    return {bounds[2] - bounds[0], bounds[3] - bounds[1]};
}

Vector2 Canvas2D::textBoxBounds(const float width, const std::string::value_type* start,
                                const std::string::value_type* end) const {
    float bounds[4];
    nvgTextBoxBounds(vg, 0.0f, 0.0f, width, start, end, bounds);
    return {bounds[2], bounds[3]};
}
