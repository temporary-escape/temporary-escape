#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Path.hpp"
#include "Texture2D.hpp"
#include <glad/glad.h>
#include <memory>
#include <unordered_map>

struct NVGcontext;

namespace Engine {
class ENGINE_API Canvas2D {
public:
    Canvas2D();
    explicit Canvas2D(const NoCreate&);
    virtual ~Canvas2D();

    using FontHandle = int;
    using ImageHandle = int;

    struct Image {
        ImageHandle handle{0};
        Vector2i atlasSize;
        Vector2i size;
        Vector2i pos;
    };

    void beginFrame(const Vector2i& viewport);
    void endFrame();
    void scissor(const Vector2& pos, const Vector2& size) const;
    void beginPath() const;
    void closePath() const;
    void roundedRect(const Vector2& pos, const Vector2& size, float radius) const;
    void rect(const Vector2& pos, const Vector2& size) const;
    void circle(const Vector2& pos, float radius) const;
    void moveTo(const Vector2& pos) const;
    void lineTo(const Vector2& pos) const;
    void fill() const;
    void stroke() const;
    void fillColor(const Color4& color) const;
    void strokeColor(const Color4& color) const;
    void strokeWidth(float width) const;
    FontHandle loadFont(const Path& path);
    ImageHandle textureToImageHandle(const Texture2D& texture, const Vector2i& size);
    void rectImage(const Vector2& pos, const Vector2& size, const Image& image, const Color4& color = Color4{1.0f});
    void fontFace(const FontHandle& font) const;
    void fontSize(float size) const;
    void text(const Vector2& pos, const std::string& str) const;
    void textBox(const Vector2& pos, float width, const std::string& str) const;

    Vector2 textBounds(const std::string& str) const {
        return textBounds(str.c_str(), str.c_str() + str.size());
    }

    Vector2 textBounds(const std::string::value_type* start, const std::string::value_type* end) const;

    Vector2 textBoxBounds(const float width, const std::string& str) const {
        return textBoxBounds(width, str.c_str(), str.c_str() + str.size());
    }

    Vector2 textBoxBounds(float width, const std::string::value_type* start, const std::string::value_type* end) const;

private:
    NVGcontext* vg{nullptr};
    GLint currentVao{0};
    GLint currentProgram{0};
    std::unordered_map<std::string, FontHandle> fontCache;
};
} // namespace Engine
