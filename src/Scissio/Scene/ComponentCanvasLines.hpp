#pragma once

#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"

namespace Scissio {
class SCISSIO_API ComponentCanvasLines : public Component {
public:
    ComponentCanvasLines() = default;
    explicit ComponentCanvasLines(Object& object, const float width, const Color4& color)
        : Component(object), width(width), color(color) {
    }
    virtual ~ComponentCanvasLines() = default;

    void add(const Vector3& from, const Vector3& to) {
        lines.emplace_back(from, to);
    }

    [[nodiscard]] const std::vector<std::tuple<Vector3, Vector3>>& getLines() const {
        return lines;
    }

    float getWidth() const {
        return width;
    }

    const Color4& getColor() const {
        return color;
    }

private:
    float width;
    Color4 color;
    std::vector<std::tuple<Vector3, Vector3>> lines;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Scissio
