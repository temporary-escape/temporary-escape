#pragma once

#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Path.hpp"
#include "Texture2D.hpp"
#include <glad/glad.h>
#include <memory>
#include <unordered_map>

struct NVGcontext;

namespace Scissio {
class FontFace;
class Image;
class Icon;

class SCISSIO_API Canvas2D {
public:
    Canvas2D();
    explicit Canvas2D(NoCreate&);
    virtual ~Canvas2D();

    using FontHandle = int;

    class Image {
    public:
        Image() : handle(-1), atlasSize{0}, pos{0}, size{0} {
        }

        explicit Image(const int handle, const Vector2& atlasSize, const Vector2& pos, const Vector2& size)
            : handle(handle), atlasSize(atlasSize), pos(pos), size(size) {
        }

        ~Image() = default;

        int getHandle() const {
            return handle;
        }

        const Vector2& getPos() const {
            return pos;
        }

        const Vector2& getSize() const {
            return size;
        }

        const Vector2& getAtlasSize() const {
            return atlasSize;
        }

    private:
        int handle;
        Vector2 atlasSize;
        Vector2 pos;
        Vector2 size;
    };

    void beginFrame(const Vector2i& viewport);
    void endFrame();
    void scissor(const Vector2& pos, const Vector2& size) const;
    void beginPath() const;
    void closePath() const;
    void roundedRect(const Vector2& pos, const Vector2& size, float radius) const;
    void rect(const Vector2& pos, const Vector2& size) const;
    void moveTo(const Vector2& pos) const;
    void lineTo(const Vector2& pos) const;
    void fill() const;
    void stroke() const;
    void fillColor(const Color4& color) const;
    void strokeColor(const Color4& color) const;
    void strokeWidth(float width) const;
    FontHandle loadFont(const Path& path);
    /*Image loadImage(const Texture2D& texture, const Vector2i& textureSize) {
        return loadImage(texture, textureSize, {0, 0}, textureSize);
    }*/
    // Image loadImage(const Texture2D& texture, const Vector2i& textureSize, const Vector2i& pos, const Vector2i&
    // size);
    // Image loadImage(const Scissio::Image& image);
    // Image loadImage(const Scissio::Icon& icon);
    // void rectImage(const Vector2& pos, const Vector2& size, const Image& image,
    //               const Color4& color = Color4{1.0f}) const;
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
} // namespace Scissio
